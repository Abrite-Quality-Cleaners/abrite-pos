#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>

class User : public QObject {
    Q_OBJECT

public:
    static User &instance();

    QString getUsername() const;
    bool isAdmin() const;

    void setUsername(const QString &username);
    void setAdmin(bool isAdmin);

signals:
    void userUpdated(); // Signal emitted when the user is updated

private:
    explicit User(const QString &username = "", bool isAdmin = false);

    QString username;
    bool admin;

    // Delete copy constructor and assignment operator
    User(const User &) = delete;
    User &operator=(const User &) = delete;
};

#endif // USER_H