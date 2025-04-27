#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>

class User {
public:
    User(const QString &username = "", bool isAdmin = false, const QString &password = "")
        : username(username), admin(isAdmin), password(password) {};

    QString getUsername() const { return username; };
    QString getPassword() const { return password; };
    bool        isAdmin() const { return admin;    };

    void setUsername(const QString &username) { this->username = username; };
    void setPassword(const QString &password) { this->password = password; };
    void    setAdmin(bool isAdmin)            { this->admin    = isAdmin;  };

private:
    QString username;
    bool admin;
    QString password;
};

#endif // USER_H