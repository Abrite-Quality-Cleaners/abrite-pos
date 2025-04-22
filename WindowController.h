#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QObject>

class LoginWindow;
class StoreSelectionWindow;
class ClientSelectionWindow;
class DropoffWindow;

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
    void onDropoffDone(); // Slot for handling the dropoffDone signal

private:
    LoginWindow *loginWindow;
    StoreSelectionWindow *storeWindow;
    ClientSelectionWindow *clientSelWindow;
    DropoffWindow *dropoffWindow;
};

#endif // WINDOWCONTROLLER_H