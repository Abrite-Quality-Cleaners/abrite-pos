#include "Session.h"
#include "WindowController.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WindowController winController;
    winController.start();
    return a.exec();
}
