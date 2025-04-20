#include "LoginWindow.h"
#include "User.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    usernameEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Login", this);

    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false); // hidden by default

    layout->addWidget(new QLabel("Username:"));
    layout->addWidget(usernameEdit);
    layout->addWidget(new QLabel("Password:"));
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);
    layout->addWidget(errorLabel);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    // Example credentials (replace with file/database lookup)
    if (username == "admin" && password == "password") {
        User::instance().setUsername(username);
        User::instance().setAdmin(true);
        errorLabel->setVisible(false);  // hide previous error
        emit loginSuccess();
    } else if (username == "user" && password == "password") {
        User::instance().setUsername(username);
        User::instance().setAdmin(false);
        errorLabel->setVisible(false);  // hide previous error
        emit loginSuccess();
    } else {
        errorLabel->setText("Invalid username or password.");
        errorLabel->setVisible(true);
    }
}