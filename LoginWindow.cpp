#include "LoginWindow.h"
#include "User.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QSettings>

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

    loadUsers();
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    // Check if the username exists in the map
    if (users.contains(username)) {
        QPair<QString, bool> userInfo = users.value(username);
        QString storedPassword = userInfo.first;
        bool isAdmin = userInfo.second;

        if (password == storedPassword) {
            User::instance().setUsername(username);
            User::instance().setAdmin(isAdmin);
            errorLabel->setVisible(false);  // hide previous error
            emit loginSuccess();
            return;
        }
    }

    // If credentials are invalid
    errorLabel->setText("Invalid username or password.");
    errorLabel->setVisible(true);
}

void LoginWindow::loadUsers()
{
    QSettings settings("../users.ini", QSettings::IniFormat);

    // Clear the map in case this method is called multiple times
    users.clear();

    // Iterate through all groups (usernames)
    QStringList usernames = settings.childGroups();
    for (const QString &username : usernames) {
        settings.beginGroup(username);
        QString password = settings.value("password").toString();
        bool isAdmin = settings.value("isAdmin").toBool();
        users.insert(username, qMakePair(password, isAdmin));
        settings.endGroup();

        // Print the username
        qDebug() << "User loaded:" << username;
    }
}