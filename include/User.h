#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>

class User : public QObject {
    Q_OBJECT

public:
    User(const QString &username = "", bool isAdmin = false);
    User(const User &other); // Copy constructor
    User &operator=(const User &other); // Assignment operator declaration

    QString getUsername() const;
    bool isAdmin() const;

    void setUsername(const QString &username);
    void setAdmin(bool isAdmin);

signals:
    void userUpdated();

private:
    QString username;
    bool admin;
};

#endif // USER_H