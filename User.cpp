#include "User.h"

// Singleton instance
User &User::instance() {
    static User instance;
    return instance;
}

User::User(const QString &username, bool isAdmin)
    : username(username), admin(isAdmin) {}

QString User::getUsername() const {
    return username;
}

bool User::isAdmin() const {
    return admin;
}

void User::setUsername(const QString &username) {
    this->username = username;
}

void User::setAdmin(bool isAdmin) {
    this->admin = isAdmin;
}