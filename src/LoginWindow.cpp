#include "LoginWindow.h"
#include "User.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QDebug>
#include <QSettings>
#include "Session.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);

    // Create widgets
    QLabel *usernameLabel = new QLabel("Username:", this);
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Enter your username");

    QLabel *passwordLabel = new QLabel("Password:", this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password); // Hide password input

    loginButton = new QPushButton("Login", this);

    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false); // Hidden by default

    // Add spacer items
    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 0, 1, 3); // Top spacer
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 0);       // Left spacer

    // Add widgets to the grid
    layout->addWidget(usernameLabel, 1, 2);
    layout->addWidget(usernameEdit, 2, 2);
    layout->addWidget(passwordLabel, 3, 2);
    layout->addWidget(passwordEdit, 4, 2);
    layout->addWidget(loginButton, 5, 2);
    layout->addWidget(errorLabel, 6, 2);

    // Add bottom and right spacers
    layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 3);       // Right spacer
    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding), 7, 0, 1, 3); // Bottom spacer

    setLayout(layout);

    // Set window size
    resize(1280, 1024);

    // Connect the login button to the login slot
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);

    // Connect pressing Enter in the password box to the login slot
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);

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
            User user(username, isAdmin);
            Session::instance().setUser(user);
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