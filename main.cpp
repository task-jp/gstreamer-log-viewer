#include "mainwindow.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("GStreamer Log Viewer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("Signal Slot. Inc,"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("signal-slot.co.jp"));

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
