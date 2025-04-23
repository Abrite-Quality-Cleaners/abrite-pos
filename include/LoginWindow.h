#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QMap>

class QPushButton;
class QLineEdit;
class QLabel;

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginSuccess();

private slots:
    void onLoginClicked();

private:
    void loadUsers(); // New method to load users into memory

    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *errorLabel;

    QMap<QString, QPair<QString, bool>> users; // Stores username -> (password, isAdmin)
};

#endif // LOGINWINDOW_H