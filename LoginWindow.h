#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

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
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *errorLabel;
};

#endif // LOGINWINDOW_H