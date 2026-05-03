#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Morse Code Translator");
    app.setOrganizationName("DSA Project");
    app.setStyle("Fusion");

    MainWindow w;
    w.show();

    return app.exec();
}
