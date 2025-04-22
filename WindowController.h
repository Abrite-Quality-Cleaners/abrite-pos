#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QObject>

class LoginWindow;
class StoreSelectionWindow;
class ClientSelectionWindow;
class DropoffWindow;
class PickupWindow;

class WindowController : public QObject {
    Q_OBJECT

public:
    explicit WindowController(QObject *parent = nullptr);
    void start();

private slots:
    void onLoginSuccess();
    void onStoreSelected();
    void onLogoutRequested();
    void onDropOffRequested();
    void onPickUpRequested(); // Slot for handling the pickup button click
    void onDropoffDone();
    void onPickupDone(); // Slot for handling the pickupDone signal

private:
    LoginWindow *loginWindow;
    StoreSelectionWindow *storeWindow;
    ClientSelectionWindow *clientSelWindow;
    DropoffWindow *dropoffWindow;
    PickupWindow *pickupWindow;
};

#endif // WINDOWCONTROLLER_H