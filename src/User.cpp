#include "User.h"
#include <QDebug>

// Singleton instance
User &User::instance() {
    static User instance;
    return instance;
}

User::User(const QString &username, bool isAdmin)
    : QObject(nullptr), username(username), admin(isAdmin) {}

QString User::getUsername() const {
    return username;
}

bool User::isAdmin() const {
    return admin;
}

void User::setUsername(const QString &username) {
    this->username = username;
    qDebug() << "Username set to:" << username;
    emit userUpdated(); // Emit signal
}

void User::setAdmin(bool isAdmin) {
    this->admin = isAdmin;
    emit userUpdated(); // Emit signal
}