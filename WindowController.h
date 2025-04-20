#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QObject>

// Forward declarations of your custom windows
class LoginWindow;
class StoreSelectionWindow;

class WindowController : public QObject {
    Q_OBJECT

public:
    explicit WindowController(QObject *parent = nullptr);
    void start();

private:
    LoginWindow          *loginWindow;
    StoreSelectionWindow *storeWindow;

private slots:
    void onLoginSuccess();
    void onStoreSelected();
};

#endif // WINDOWCONTROLLER_H