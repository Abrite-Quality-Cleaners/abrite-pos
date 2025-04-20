#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QObject>
#include "ClientSelectionWindow.h"
#include "DropoffWindow.h"

// Forward declarations of your custom windows
class LoginWindow;
class StoreSelectionWindow;

class WindowController : public QObject
{
    Q_OBJECT

public:
    explicit WindowController(QObject *parent = nullptr);
    void start();

private slots:
    void onLoginSuccess();
    void onStoreSelected();
    void onLogoutRequested(); // Slot to handle logout
    void onDropOffRequested(); // Slot to handle Drop-off button click

private:
    LoginWindow           *loginWindow;
    StoreSelectionWindow  *storeWindow;
    ClientSelectionWindow *clientSelWindow;
    DropoffWindow         *dropoffWindow;
};

#endif // WINDOWCONTROLLER_H