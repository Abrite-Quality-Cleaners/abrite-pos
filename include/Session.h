#ifndef SESSION_H
#define SESSION_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <memory>
#include <QDebug>
#include "User.h"
#include "MongoManager.h"

class Session : public QObject {
    Q_OBJECT

public:
    static Session& instance() {
        static Session instance;
        return instance;
    }

signals:
    void customerUpdated();
    void userUpdated();

public:
    // User-related methods
    const User& getUser() const { return user; }
    void setUser(const User& userObj) { user = userObj; emit userUpdated(); }

    // Store-related methods
    QString getStoreName() const { return storeName; }
    void setStoreName(const QString& name) { storeName = name; }

    // Customer-related methods
    QMap<QString, QVariant> getCustomer() const { return customer; }
    void setCustomer(const QMap<QString, QVariant>& customerData) { customer = customerData; emit customerUpdated(); }

    // Database-related methods
    MongoManager& getMongoManager(const QString &connectionString = "mongodb://localhost:27017", 
                                  const QString &dbName = "default-database") {
        if (!mongoManager) {
            mongoManager = std::make_unique<MongoManager>(connectionString, dbName);
            qDebug() << "MongoManager created with database:" << dbName;
        }
        return *mongoManager;
    }

private:
    Session() = default;
    ~Session() = default;

    // Disable copy and assignment
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    User user;
    QString storeName;
    QMap<QString, QVariant> customer;
    std::unique_ptr<MongoManager> mongoManager;
};

#endif // SESSION_H