#include "mainwindow.h"
#include "constant.h"
#include "language/language.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load(Language::instance()->getCurrentQM());
    qApp->installTranslator(&translator);

    // change app font family and size to supprot all device.
    QFont appFont = app.font();
    appFont.setPixelSize(font_size);
    app.setFont(appFont);

    MainWindow w;

#ifdef PLATFORM_WAYLAND
    w.showFullScreen();
#else
    w.showMaximized();
#endif

    return app.exec();
}
