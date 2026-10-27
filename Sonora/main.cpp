#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/Sonora.png"));

    app.setApplicationName("Music Player");
    app.setOrganizationName("MusicPlayer");
    app.setApplicationDisplayName("Sonora");

    QFont appFont("Segoe UI", 10);
    app.setFont(appFont);

    MainWindow window;
    window.show();

    return app.exec();
}