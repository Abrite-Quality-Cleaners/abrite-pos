#ifndef SESSION_H
#define SESSION_H

#include <QString>
#include <QMap>
#include <QVariant>

class Session : public QObject {
    Q_OBJECT

public:
    static Session& instance() {
        static Session instance;
        return instance;
    }

signals:
    void customerUpdated();

public:
    // User-related methods
    QString getUserName() const { return userName; }
    void setUserName(const QString& name) { userName = name; }

    // Store-related methods
    QString getStoreName() const { return storeName; }
    void setStoreName(const QString& name) { storeName = name; }

    // Customer-related methods
    QMap<QString, QVariant> getCustomer() const { return customer; }
    void setCustomer(const QMap<QString, QVariant>& customerData) { customer = customerData; emit customerUpdated(); }

private:
    Session() = default;
    ~Session() = default;

    // Disable copy and assignment
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    QString userName;
    QString storeName;
    QMap<QString, QVariant> customer;
};

#endif // SESSION_H