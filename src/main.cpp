#include "MainWindow.h" // Include our window header
#include <QApplication> // Qt Application class
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "DEBUG: Launching Qt Application..." << std::endl;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int result = a.exec();
    std::cout << "DEBUG: Qt Application finished." << std::endl;
    return result;
}