#include "User.h"

User::User(const QString &username, bool isAdmin)
    : QObject(nullptr), username(username), admin(isAdmin) {}

User::User(const User &other)
    : QObject(nullptr), username(other.username), admin(other.admin) {} // Copy constructor implementation

User &User::operator=(const User &other) {
    if (this != &other) {
        this->username = other.username;
        this->admin = other.admin;
        // QObject parent is not copied to avoid QObject-related issues
    }
    return *this;
}

QString User::getUsername() const {
    return username;
}

bool User::isAdmin() const {
    return admin;
}

void User::setUsername(const QString &username) {
    this->username = username;
    emit userUpdated();
}

void User::setAdmin(bool isAdmin) {
    this->admin = isAdmin;
    emit userUpdated();
}