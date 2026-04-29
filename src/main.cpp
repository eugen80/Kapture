#include <QApplication>

#include "CaptureOverlay.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CaptureOverlay overlay;
    overlay.showFullScreen();

    return app.exec();
}
