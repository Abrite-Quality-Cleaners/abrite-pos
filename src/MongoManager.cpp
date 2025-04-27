#include "MongoManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>
#include "Customer.h"
#include "Address.h"
#include "Order.h"

MongoManager::MongoManager(const QString &connectionString, const QString &dbName, QObject *parent)
    : QObject(parent), connectionString(connectionString), dbName(dbName), client(mongocxx::uri(connectionString.toStdString())) {
    database = client[dbName.toStdString()];
    qDebug() << "Connected to MongoDB database:" << dbName;
}

MongoManager::~MongoManager() {
    qDebug() << "Closing MongoDB connection.";
}

// Helper: Convert QMap to BSON
bsoncxx::document::value MongoManager::toBson(const QMap<QString, QVariant> &data) {
    bsoncxx::builder::stream::document doc;
    for (auto it = data.begin(); it != data.end(); ++it) {
        const QString &key = it.key();
        const QVariant &value = it.value();

        if (value.metaType().id() == QMetaType::QVariantMap) {
            // Handle nested objects
            doc << key.toStdString() << toBson(value.toMap()).view();
        } else if (value.metaType().id() == QMetaType::QVariantList) {
            // Handle arrays
            bsoncxx::builder::basic::array arrayBuilder;
            for (const QVariant &item : value.toList()) {
                if (item.metaType().id() == QMetaType::QVariantMap) {
                    arrayBuilder.append(toBson(item.toMap()).view());
                } else if (item.metaType().id() == QMetaType::Double) {
                    arrayBuilder.append(item.toDouble());
                } else if (item.metaType().id() == QMetaType::Int) {
                    arrayBuilder.append(item.toInt());
                } else {
                    arrayBuilder.append(item.toString().toStdString());
                }
            }
            doc << key.toStdString() << arrayBuilder.view();
        } else if (value.metaType().id() == QMetaType::Double) {
            // Handle double values
            doc << key.toStdString() << value.toDouble();
        } else if (value.metaType().id() == QMetaType::Int) {
            // Handle int values
            doc << key.toStdString() << value.toInt();
        } else {
            // Handle primitive types (default to string)
            doc << key.toStdString() << value.toString().toStdString();
        }
    }
    return doc.extract();
}

// Helper: Convert BSON to QMap
QMap<QString, QVariant> MongoManager::fromBson(const bsoncxx::document::view &doc) {
    QMap<QString, QVariant> data;
    for (auto element : doc) {
        QString key = QString::fromStdString(std::string(element.key()));

        if (element.type() == bsoncxx::type::k_document) {
            // Handle nested objects
            data[key] = fromBson(element.get_document().view());
        } else if (element.type() == bsoncxx::type::k_array) {
            // Handle arrays
            QVariantList list;
            for (auto arrayElement : element.get_array().value) {
                if (arrayElement.type() == bsoncxx::type::k_document) {
                    list.append(fromBson(arrayElement.get_document().view()));
                } else if (arrayElement.type() == bsoncxx::type::k_string) {
                    list.append(QString::fromStdString(std::string(arrayElement.get_string().value)));
                } else if (arrayElement.type() == bsoncxx::type::k_double) {
                    list.append(arrayElement.get_double().value);
                } else if (arrayElement.type() == bsoncxx::type::k_int32) {
                    list.append(arrayElement.get_int32().value);
                } else if (arrayElement.type() == bsoncxx::type::k_int64) {
                    list.append(static_cast<qlonglong>(arrayElement.get_int64().value));
                }
            }
            data[key] = list;
        } else if (element.type() == bsoncxx::type::k_string) {
            // Handle strings
            data[key] = QString::fromStdString(std::string(element.get_string().value));
        } else if (element.type() == bsoncxx::type::k_double) {
            // Handle double values
            data[key] = element.get_double().value;
        } else if (element.type() == bsoncxx::type::k_int32) {
            // Handle int32 values
            data[key] = element.get_int32().value;
        } else if (element.type() == bsoncxx::type::k_int64) {
            // Handle int64 values
            data[key] = static_cast<qlonglong>(element.get_int64().value);
        }
    }
    return data;
}

// Add a new customer
QString MongoManager::addCustomer(const QMap<QString, QVariant> &customerData) {
    // Validate required fields
    if (!customerData.contains("firstName") || !customerData.contains("lastName")) {
        qDebug() << "Error: Missing required fields for customer.";
        return QString();
    }

    try {
        auto collection = database["customers"];
        auto result = collection.insert_one(toBson(customerData));
        if (result.has_value()) {
            return QString::fromStdString(result->inserted_id().get_oid().value.to_string());
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error adding customer:" << e.what();
    }
    return QString();
}

// Get a customer by ID
QMap<QString, QVariant> MongoManager::getCustomer(const QString &customerId) {
    try {
        auto collection = database["customers"];
        auto result = collection.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(customerId.toStdString()) << bsoncxx::builder::stream::finalize);
        if (result) {
            return fromBson(result->view());
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error fetching customer:" << e.what();
    }
    return {};
}

// Update a customer
bool MongoManager::updateCustomer(const QString &customerId, const QMap<QString, QVariant> &updatedData) {
    try {
        auto collection = database["customers"];
        auto result = collection.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(customerId.toStdString()) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$set" << toBson(updatedData).view() << bsoncxx::builder::stream::finalize);
        return result && result->modified_count() > 0;
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error updating customer:" << e.what();
    }
    return false;
}

// Delete a customer
bool MongoManager::deleteCustomer(const QString &customerId) {
    try {
        auto collection = database["customers"];
        auto result = collection.delete_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(customerId.toStdString()) << bsoncxx::builder::stream::finalize);
        return result && result->deleted_count() > 0;
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error deleting customer:" << e.what();
    }
    return false;
}

// Add a new order
QString MongoManager::addOrder(const QMap<QString, QVariant> &orderData) {
    // Validate required fields
    if (!orderData.contains("customerId") || !orderData.contains("orderItems")) {
        qDebug() << "Error: Missing required fields for order.";
        return QString();
    }

    // Validate orderItems structure
    QVariantList orderItems = orderData["orderItems"].toList();
    for (const QVariant &item : orderItems) {
        QMap<QString, QVariant> category = item.toMap();
        if (!category.contains("category") || !category.contains("items")) {
            qDebug() << "Error: Invalid orderItems structure.";
            return QString();
        }
    }

    try {
        auto collection = database["orders"];
        auto result = collection.insert_one(toBson(orderData));
        if (result.has_value()) {
            return QString::fromStdString(result->inserted_id().get_oid().value.to_string());
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error adding order:" << e.what();
    }
    return QString();
}

// Get an order by ID
QMap<QString, QVariant> MongoManager::getOrder(const QString &orderId) {
    try {
        auto collection = database["orders"];
        auto result = collection.find_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(orderId.toStdString()) << bsoncxx::builder::stream::finalize);
        if (result) {
            return fromBson(result->view());
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error fetching order:" << e.what();
    }
    return {};
}

// Get all orders for a customer
QList<QMap<QString, QVariant>> MongoManager::getOrdersByCustomer(const QString &customerId) {
    QList<QMap<QString, QVariant>> orders;
    try {
        auto collection = database["orders"];
        auto cursor = collection.find(bsoncxx::builder::stream::document{} << "customerId" << bsoncxx::oid(customerId.toStdString()) << bsoncxx::builder::stream::finalize);
        for (auto doc : cursor) {
            orders.append(fromBson(doc));
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error fetching orders:" << e.what();
    }
    return orders;
}

// Update an order
bool MongoManager::updateOrder(const QString &orderId, const QMap<QString, QVariant> &updatedData) {
    try {
        auto collection = database["orders"];
        auto result = collection.update_one(
            bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(orderId.toStdString()) << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$set" << toBson(updatedData).view() << bsoncxx::builder::stream::finalize);
        return result && result->modified_count() > 0;
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error updating order:" << e.what();
    }
    return false;
}

// Delete an order
bool MongoManager::deleteOrder(const QString &orderId) {
    try {
        auto collection = database["orders"];
        auto result = collection.delete_one(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::oid(orderId.toStdString()) << bsoncxx::builder::stream::finalize);
        return result && result->deleted_count() > 0;
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error deleting order:" << e.what();
    }
    return false;
}

mongocxx::database& MongoManager::getDatabase() {
    return database;
}

// Dump the contents of a specific collection
void MongoManager::dumpCollection(const QString &collectionName) const {
    try {
        auto collection = database[collectionName.toStdString()];
        auto cursor = collection.find({});
        qDebug() << "Contents of collection:" << collectionName;
        for (const auto &doc : cursor) {
            qDebug().noquote() << QString::fromStdString(bsoncxx::to_json(doc));
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error dumping collection:" << e.what();
    }
}

// Dump the contents of the entire database
void MongoManager::dumpDatabase() {
    try {
        auto collections = database.list_collections();
        for (const auto &collection : collections) {
            QString collectionName = QString::fromStdString(std::string(collection["name"].get_string().value));
            dumpCollection(collectionName);
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error dumping database:" << e.what();
    }
}

QString MongoManager::addCustomer(const Customer &customer) {
    QMap<QString, QVariant> customerData = {
        {"firstName", customer.firstName},
        {"lastName", customer.lastName},
        {"phoneNumber", customer.phoneNumber},
        {"email", customer.email},
        {"address", QMap<QString, QVariant>{
            {"street", customer.address.street},
            {"city", customer.address.city},
            {"state", customer.address.state},
            {"zip", customer.address.zip}
        }},
        {"note", customer.note},
        {"balance", customer.balance},
        {"storeCreditBalance", customer.storeCreditBalance}
    };
    return addCustomer(customerData);
}

Customer MongoManager::getCustomerById(const QString &customerId) {
    QMap<QString, QVariant> data = getCustomer(customerId);
    Customer customer;
    customer.id = customerId;
    customer.firstName = data["firstName"].toString();
    customer.lastName = data["lastName"].toString();
    customer.phoneNumber = data["phoneNumber"].toString();
    customer.email = data["email"].toString();
    QMap<QString, QVariant> addressData = data["address"].toMap();
    customer.address = Address(
        addressData["street"].toString(),
        addressData["city"].toString(),
        addressData["state"].toString(),
        addressData["zip"].toString()
    );
    customer.note = data["note"].toString();
    customer.balance = data["balance"].toDouble();
    customer.storeCreditBalance = data["storeCreditBalance"].toDouble();
    return customer;
}

bool MongoManager::updateCustomer(const Customer &customer) {
    QMap<QString, QVariant> updatedData = {
        {"firstName", customer.firstName},
        {"lastName", customer.lastName},
        {"phoneNumber", customer.phoneNumber},
        {"email", customer.email},
        {"address", QMap<QString, QVariant>{
            {"street", customer.address.street},
            {"city", customer.address.city},
            {"state", customer.address.state},
            {"zip", customer.address.zip}
        }},
        {"note", customer.note},
        {"balance", customer.balance},
        {"storeCreditBalance", customer.storeCreditBalance}
    };
    return updateCustomer(customer.id, updatedData);
}

QString MongoManager::addOrder(const Order &order) {
    QMap<QString, QVariant> orderData = {
        {"customerId", order.customerId},
        {"store", order.store},
        {"orderItems", QVariant::fromValue(order.orderItems)},
        {"orderTotal", order.orderTotal},
        {"status", order.status},
        {"ticketNumber", order.ticketNumber},
        {"dropoffDate", order.dropoffDate},
        {"dropoffEmployee", order.dropoffEmployee},
        {"pickupDate", order.pickupDate},
        {"pickupEmployee", order.pickupEmployee},
        {"paymentDate", order.paymentDate},
        {"paymentType", order.paymentType},
        {"paymentEmployee", order.paymentEmployee},
        {"voidDate", order.voidDate},
        {"voidEmployee", order.voidEmployee},
        {"orderNote", order.orderNote},
        {"rackNumber", order.rackNumber},
        {"orderReadyDate", order.orderReadyDate}
    };
    return addOrder(orderData);
}

Order MongoManager::getOrderById(const QString &orderId) {
    QMap<QString, QVariant> data = getOrder(orderId);
    Order order;
    order.id = orderId;
    order.customerId = data["customerId"].toString();
    order.store = data["store"].toString();
    order.orderItems = data["orderItems"].value<QList<OrderCategory>>();
    order.orderTotal = data["orderTotal"].toDouble();
    order.status = data["status"].toString();
    order.ticketNumber = data["ticketNumber"].toString();
    order.dropoffDate = data["dropoffDate"].toString();
    order.dropoffEmployee = data["dropoffEmployee"].toString();
    order.pickupDate = data["pickupDate"].toString();
    order.pickupEmployee = data["pickupEmployee"].toString();
    order.paymentDate = data["paymentDate"].toString();
    order.paymentType = data["paymentType"].toString();
    order.paymentEmployee = data["paymentEmployee"].toString();
    order.voidDate = data["voidDate"];
    order.voidEmployee = data["voidEmployee"];
    order.orderNote = data["orderNote"].toString();
    order.rackNumber = data["rackNumber"].toString();
    order.orderReadyDate = data["orderReadyDate"].toString();
    return order;
}