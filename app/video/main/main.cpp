#include "mainwindow.h"
#include "constant.h"
#include "language.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(Language::instance()->getCurrentQM()))
        qApp->installTranslator(&translator);
    else
        qDebug("load translator failed.");

    // change app font family and size to supprot all device.
    QFont appFont = app.font();
    appFont.setPixelSize(font_size);
    app.setFont(appFont);

    MainWindow w;
    w.showFullScreen();

    return app.exec();
}
