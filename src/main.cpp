#include "Session.h"
#include "WindowController.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WindowController winController;
    winController.start();

    Session::instance().setUserName("Example User");
    Session::instance().setStoreName("Example Store");
    Session::instance().setCustomer({{"firstName", "John"}, {"lastName", "Doe"}});

    QString userName = Session::instance().getUserName();
    QString storeName = Session::instance().getStoreName();
    QMap<QString, QVariant> customer = Session::instance().getCustomer();

    return a.exec();
}
