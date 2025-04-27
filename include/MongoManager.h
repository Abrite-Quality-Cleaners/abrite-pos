#ifndef MONGOMANAGER_H
#define MONGOMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QVariant>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <bsoncxx/json.hpp>
#include "Customer.h" // Include the Customer header file
#include "Order.h" // Include the Order header file

class MongoManager : public QObject {
    Q_OBJECT

public:
    explicit MongoManager(const QString &connectionString, const QString &dbName, QObject *parent = nullptr);
    ~MongoManager();

    // Customer operations
    QString addCustomer(const QMap<QString, QVariant> &customerData);
    QString addCustomer(const Customer &customer);
    QMap<QString, QVariant> getCustomer(const QString &customerId);
    Customer getCustomerById(const QString &customerId);
    bool updateCustomer(const QString &customerId, const QMap<QString, QVariant> &updatedData);
    bool updateCustomer(const Customer &customer);
    bool deleteCustomer(const QString &customerId);

    // Order operations
    QString addOrder(const QMap<QString, QVariant> &orderData);
    QString addOrder(const Order &order);
    QMap<QString, QVariant> getOrder(const QString &orderId);
    Order getOrderById(const QString &orderId);
    QList<QMap<QString, QVariant>> getOrdersByCustomer(const QString &customerId);
    bool updateOrder(const QString &orderId, const QMap<QString, QVariant> &updatedData);
    bool deleteOrder(const QString &orderId);

    // Getter for the database
    mongocxx::database& getDatabase();

    void dumpCollection(const QString &collectionName) const;
    void dumpDatabase();

private:
    mongocxx::instance instance; // MongoDB driver instance
    mongocxx::client client;     // MongoDB client
    mongocxx::database database; // MongoDB database

    QString connectionString;
    QString dbName;

    bsoncxx::document::value toBson(const QMap<QString, QVariant> &data);
    QMap<QString, QVariant> fromBson(const bsoncxx::document::view &doc);
};

#endif // MONGOMANAGER_H