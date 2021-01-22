#include "updaterwidget.h"
#include "ui_updaterwidget.h"
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QCryptographicHash>
#include <QtConcurrent/QtConcurrent>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stropts.h>

#include "constant.h"
#include "language/language.h"


int vendor_storage_write(int buf_size, uint8 *buf, uint16 vendor_id)
{
    int ret = 0;
    int fd;
    RK_VERDOR_REQ req;

    fd = open(VERDOR_DEVICE, O_RDWR, 0);
    if (fd < 0) {
        printf("vendor_storage open fail, errno = %d\n", errno);
        return -1;
    }
    req.tag = VENDOR_REQ_TAG;
    req.id = vendor_id;
    req.len = buf_size > VENDOR_DATA_SIZE ? VENDOR_DATA_SIZE : buf_size;
    memcpy(req.data, buf, req.len);
    ret = ioctl(fd, VENDOR_WRITE_IO, &req);
    if (ret) {
        printf("vendor write error, ret = %d\n", ret);
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

int vendor_storage_read(int buf_size, uint8 *buf, uint16 vendor_id)
{
    int ret = 0;
    int fd;
    RK_VERDOR_REQ req;

    fd = open(VERDOR_DEVICE, O_RDWR, 0);
    if (fd < 0) {
        printf("vendor_storage open fail, errno = %d\n", errno);
        return -1;
    }
    req.tag = VENDOR_REQ_TAG;
    req.id = vendor_id;
    req.len = buf_size > VENDOR_DATA_SIZE ? VENDOR_DATA_SIZE : buf_size;
    ret = ioctl(fd, VENDOR_READ_IO, &req);
    if (ret) {
        printf("vendor read error, ret = %d\n", ret);
        close(fd);
        return -1;
    }
    close(fd);
    memcpy(buf, req.data, req.len);

    return 0;
}


int fw_flag_check(char* path)
{
    int fdfile = 0;
    int ret = 0;
    STRUCT_PART_INFO partition;

    fdfile = open(path, O_RDONLY);
    if (fdfile <= 0) {
        printf("fw_flag_check open %s failed! \n", path);
        perror("open");
        return -1;
    }

    ret = read(fdfile, &partition, sizeof(STRUCT_PART_INFO));
    if (ret <= 0) {
        close(fdfile);
        printf("fw_flag_check read %s failed! \n", path);
        perror("read");
        return -1;
    }
    close(fdfile);

    if (partition.hdr.uiFwTag != RK_PARTITION_TAG) {
        printf("ERROR: Your firmware(%s) is invalid!\n", path);
        return -1;
    }

    return 0;
}

int UpdaterWidget::fw_md5_check(QString firmwarePath)
{
    QString md5;
    QString firmwareMd5Path = firmwarePath;
    QFile hashFile(firmwareMd5Path.replace(".img", ".md5"));
    if (hashFile.exists()) {
        hashFile.open(QIODevice::ReadOnly);
        QStringList list = QString(QLatin1String(hashFile.readAll().constData())).trimmed().split(" ");
        if (list.length() > 0)
            md5= list.at(0);

        hashFile.close();
        qDebug() << firmwareMd5Path << md5;
    }

    if (md5 == "") {
        ui->m_textBrowser->append(firmwareMd5Path + " not found.");
        return 1;
    }

    ui->m_textBrowser->append("Computing the hash");

    QFile theFile(firmwarePath);
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
    QString firmwareMd5 = QString(QLatin1String((ba.toHex().constData()))).trimmed();
    theFile.close();

    qDebug() << " md5:" << md5 << "firmwareMd5:" << firmwareMd5;
    ui->m_textBrowser->append("md5:" + firmwareMd5);

    if (firmwareMd5 == md5)
        return 0;


    return 1;
}

UpdaterWidget::UpdaterWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::UpdaterWidget)
  , state(false)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(finish()));
    pIndicator = new QProgressIndicator(this);
    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));

    char version[255] = {0};
    sprintf(version,"%d.%d.%d", fwinfo.update_version >> 24 & 0xFF,
            fwinfo.update_version >> 16 & 0xFF, fwinfo.update_version & 0xFF);

    //read fwinfo
    int ret = vendor_storage_read(sizeof(UpdaterInfo), (unsigned char*)&fwinfo, VENDOR_UPDATER_ID);
    if (ret){
        ui->m_update_version_LineEdit->setText(QString(version));
        return;
    }

    sprintf(version,"%d.%d.%d", fwinfo.update_version >> 24 & 0xFF,
            fwinfo.update_version >> 16 & 0xFF, fwinfo.update_version & 0xFF);
    ui->m_update_version_LineEdit->setText(QString(version));
}

void UpdaterWidget::retranslateUi()
{
    ui->retranslateUi(this);
}

UpdaterWidget::~UpdaterWidget()
{
    delete ui;
}

void UpdaterWidget::on_m_updatePushButton_clicked()
{
    if (state == false) {
        pIndicator->setColor(Qt::gray);
        pIndicator->startAnimation();

        pIndicator->setFixedSize(80, 80);
        pIndicator->move(this->width() / 2 - pIndicator->width() / 2,
                         this->height() / 2 - pIndicator->height() / 2);
        pIndicator->show();
        ui->m_updatePushButton->setEnabled(false);
        QtConcurrent::run(this, &UpdaterWidget::update);

        timer->start(1000);
    }
}

void UpdaterWidget::finish()
{
    if (state) {
        pIndicator->hide();
        timer->stop();
        state = false;
        ui->m_updatePushButton->setEnabled(true);
    }
}

void UpdaterWidget::update()
{
    file = new QFileInfo("/mnt/sdcard/Firmware.img");
    QString md5;
    if (file->exists()) {
        ui->m_textBrowser->append("Found img:" + file->absoluteFilePath());
        qDebug()<<"Found img:" << file->absoluteFilePath();
    } else {
        qDebug() << file->absoluteFilePath() << " not found";
        ui->m_textBrowser->append(file->absoluteFilePath() + " not found");
        file = new QFileInfo("/mnt/udisk/Firmware.img");
        if (file->exists()) {
            ui->m_textBrowser->append("Found img:" + file->absoluteFilePath());
            qDebug() << "Found img:" << file->absoluteFilePath();
        } else {
            qDebug() << file->absoluteFilePath() << " not found";
            ui->m_textBrowser->append(file->absoluteFilePath() + " not found");
            state = true;
            return;
        }
    }

    int size =file->absoluteFilePath().toStdString().size();
    char path[size+1]={0};
    memcpy(path,file->absoluteFilePath().toStdString().c_str(),size);

    int ret = fw_flag_check(path);
    if (ret) {
        qDebug() << "fw_flag_check faild";
        ui->m_textBrowser->append("fw_flag_check faild");
        state = true;
        return;
    } else {
        qDebug()<<"fw_flag_check ok";
        ui->m_textBrowser->append("fw_flag_check ok");
    }

    ret = fw_md5_check(file->absoluteFilePath());
    state = true;
    if (ret) {
        qDebug() << "fw_md5_check faild";
        ui->m_textBrowser->append("fw_md5_check faild");
        return;
    } else {
        qDebug()<<"fw_md5_check ok";
        ui->m_textBrowser->append("fw_md5_check ok");
    }

    printf("Current Path is %s\n", fwinfo.update_path);
    printf("Upadater Path is %s\n", path);

    //write fwinfo
    fwinfo.update_mode = MODE_UPDATER;

    memset(fwinfo.update_path,0,sizeof(fwinfo.update_path));
    memcpy(fwinfo.update_path, path, strlen(path));

    ret = vendor_storage_write(sizeof(UpdaterInfo), (unsigned char*)&fwinfo, VENDOR_UPDATER_ID);

    if (ret) {
        qDebug() << "vendor_storage_write faild";
        ui->m_textBrowser->append("vendor_storage_write faild");
    } else {
        qDebug() << "vendor_storage_write ok";
        ui->m_textBrowser->append("vendor_storage_write ok");
        qDebug() << "update_version:" << fwinfo.update_version << " update_mode:" << fwinfo.update_mode
                 << " update_path:" << fwinfo.update_path;
        qDebug() << "begin reboot";
        ui->m_textBrowser->append("begin reboot");
        QProcess::execute("reboot");
    }
}

void UpdaterWidget::on_m_rstpushButton_clicked()
{
    FILE *stream = NULL;
    char buf[1024];

    ui->m_textBrowser->setPlainText("");
    ui->m_textBrowser->append("--- Factory Reset ---");

    memset(buf, '\0', sizeof(buf));
    if ((stream = popen("/usr/bin/update reset", "r")) != NULL) {
        while (fgets(buf, 1024, stream)) {
            ui->m_textBrowser->append(buf);
        }
    }

    pclose(stream);
}

void UpdaterWidget::on_m_otapushButton_clicked()
{
    FILE *stream = NULL;
    char buf[1024];

    ui->m_textBrowser->setPlainText("");
    ui->m_textBrowser->append("--- System OTA ---");

    memset(buf, '\0', sizeof(buf));
    if ((stream = popen("/usr/bin/update ota", "r")) != NULL) {
        while (fgets(buf, 1024, stream)) {
            ui->m_textBrowser->append(buf);
        }
    }

    pclose(stream);
}
