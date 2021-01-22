#include "mainwindow.h"
#include "language.h"
#include "constant.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    bool state = translator.load(Language::instance()->getCurrentQM());
    if (!state)
        qDebug("load translator failed.");
    else
        qApp->installTranslator(&translator);

    // change app font family and size to supprot all device.
    QFont appFont = app.font();
    appFont.setPixelSize(font_size);
    app.setFont(appFont);

    MainWindow w;
    w.showFullScreen();

    return app.exec();
}
