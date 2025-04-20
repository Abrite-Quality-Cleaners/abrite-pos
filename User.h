#ifndef USER_H
#define USER_H

#include <QString>

class User {
public:
    // Singleton access method
    static User &instance();

    QString getUsername() const;
    bool isAdmin() const;

    void setUsername(const QString &username);
    void setAdmin(bool isAdmin);

private:
    // Private constructor to enforce singleton
    User(const QString &username = "", bool isAdmin = false);

    QString username;
    bool admin;

    // Delete copy constructor and assignment operator
    User(const User &) = delete;
    User &operator=(const User &) = delete;
};

#endif // USER_H