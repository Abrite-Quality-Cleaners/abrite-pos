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

MongoManager::MongoManager(const QString &connectionString, const QString &dbName)
    : connectionString(connectionString), dbName(dbName), client(mongocxx::uri(connectionString.toStdString())) {
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

        if (key == "customerId") {
            // Serialize customerId as an ObjectId
            doc << key.toStdString() << bsoncxx::oid(value.toString().toStdString());
        } else if (value.metaType().id() == QMetaType::QVariantMap) {
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

        if (key == "_id" && element.type() == bsoncxx::type::k_oid) {
            // Deserialize _id as a QString
            data[key] = QString::fromStdString(element.get_oid().value.to_string());
        } else if (key == "customerId" && element.type() == bsoncxx::type::k_oid) {
            // Deserialize customerId as a QString
            data[key] = QString::fromStdString(element.get_oid().value.to_string());
        } else if (element.type() == bsoncxx::type::k_string) {
            data[key] = QString::fromStdString(std::string(element.get_string().value));
        } else if (element.type() == bsoncxx::type::k_int32) {
            data[key] = element.get_int32().value;
        } else if (element.type() == bsoncxx::type::k_int64) {
            data[key] = static_cast<qlonglong>(element.get_int64().value);
        } else if (element.type() == bsoncxx::type::k_double) {
            data[key] = element.get_double().value;
        } else if (element.type() == bsoncxx::type::k_array) {
            QVariantList list;
            for (auto arrayElement : element.get_array().value) {
                if (arrayElement.type() == bsoncxx::type::k_document) {
                    list.append(fromBson(arrayElement.get_document().value));
                }
            }
            data[key] = list;
        } else if (element.type() == bsoncxx::type::k_document) {
            data[key] = fromBson(element.get_document().view());
        }
        // Handle other BSON types as needed
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
        auto collection = database["Customers"];
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
        qDebug() << "Fetching customer with ID:" << customerId;
        auto collection = database["Customers"];
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
        auto collection = database["Customers"];
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
        auto collection = database["Customers"];
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
    if (!orderData.contains("customerId") || !orderData.contains("subOrders")) {
        qDebug() << "Error: Missing required fields for order.";
        return QString();
    }

    // Validate subOrders structure
    QVariantList subOrders = orderData["subOrders"].toList();
    for (const QVariant &item : subOrders) {
        QMap<QString, QVariant> type = item.toMap();
        if (!type.contains("type") || !type.contains("items")) {
            qDebug() << "Error: Invalid subOrders structure.";
            return QString();
        }
    }

    try {
        auto collection = database["Orders"];
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
        auto collection = database["Orders"];
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
        auto collection = database["Orders"];
        auto cursor = collection.find(bsoncxx::builder::stream::document{} 
                                      << "customerId" << bsoncxx::oid(customerId.toStdString()) 
                                      << bsoncxx::builder::stream::finalize);
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
        auto collection = database["Orders"];
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
        auto collection = database["Orders"];
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

    if (data.isEmpty()) {
        qDebug() << "No customer found with ID:" << customerId;
        return Customer("", "", "", "", "", Address("", "", "", ""), "", 0.0, 0.0); // Return an empty Customer object
    }

    // Use the Customer constructor to create the object
    return Customer(
        customerId,
        data["firstName"].toString(),
        data["lastName"].toString(),
        data["phoneNumber"].toString(),
        data["email"].toString(),
        Address(
            data["address"].toMap()["street"].toString(),
            data["address"].toMap()["city"].toString(),
            data["address"].toMap()["state"].toString(),
            data["address"].toMap()["zip"].toString()
        ),
        data["note"].toString(),
        data["balance"].toDouble(),
        data["storeCreditBalance"].toDouble()
    );
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
    QVariantList subOrdersList;
    for (const SubOrder &type : order.subOrders) {
        QVariantList itemsList;
        for (const Item &item : type.items) {
            itemsList.append(QVariantMap{
                {"name", item.name},
                {"price", item.price},
                {"quantity", item.quantity}
            });
        }
        subOrdersList.append(QVariantMap{
            {"id", static_cast<quint64>(type.id)}, // Serialize type ID
            {"type", type.type},
            {"items", itemsList},
            {"total", type.total}
        });
    }

    QMap<QString, QVariant> orderData = {
        {"customerId", order.customerId},
        {"store", order.store},
        {"subOrders", subOrdersList},
        {"orderTotal", order.orderTotal},
        {"balance", order.balance},
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

    qDebug() << "Adding order with data:" << orderData;  // Debug output
    return addOrder(orderData);
}

Order MongoManager::getOrderById(const QString &orderId) {
    QMap<QString, QVariant> data = getOrder(orderId);

    if (data.isEmpty()) {
        qDebug() << "No order found with ID:" << orderId;
        return Order();
    }

    qDebug() << "Retrieved order data:" << data;  // Debug output

    Order order;
    order.id = orderId;
    order.customerId = data["customerId"].toString();
    order.store = data["store"].toString();
    order.orderTotal = data["orderTotal"].toDouble();
    order.balance = data["balance"].toDouble();
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

    qDebug() << "Deserialized order balance:" << order.balance;  // Debug output

    QVariantList subOrdersList = data["subOrders"].toList();
    for (const QVariant &subOrderVariant : subOrdersList) {
        QMap<QString, QVariant> subOrderMap = subOrderVariant.toMap();
        SubOrder type;
        type.id = subOrderMap["id"].toULongLong(); // Deserialize type ID
        type.type = subOrderMap["type"].toString();
        type.total = subOrderMap["total"].toDouble();

        QVariantList itemsList = subOrderMap["items"].toList();
        for (const QVariant &itemVariant : itemsList) {
            QMap<QString, QVariant> itemMap = itemVariant.toMap();
            Item item;
            item.name = itemMap["name"].toString();
            item.price = itemMap["price"].toDouble();
            item.quantity = itemMap["quantity"].toInt();
            type.items.append(item);
        }

        order.subOrders.append(type);
    }

    return order;
}

QList<Customer> MongoManager::searchCustomers(const QString &firstName, 
                                              const QString &lastName, 
                                              const QString &phone, 
                                              const QString &ticket) {
    QList<Customer> customers;

    try {
        auto collection = database["Customers"];
        bsoncxx::builder::stream::document filterBuilder;

        qDebug() << "Searching customers with criteria:"
                 << "First Name:" << firstName
                 << "Last Name:" << lastName
                 << "Phone:" << phone
                 << "Ticket:" << ticket;

        // Add criteria to the filter if they are not empty
        if (!firstName.isEmpty()) {
            filterBuilder << "firstName" << bsoncxx::builder::stream::open_document
                          << "$regex" << firstName.toStdString()
                          << "$options" << "i" // Case-insensitive search
                          << bsoncxx::builder::stream::close_document;
        }
        if (!lastName.isEmpty()) {
            filterBuilder << "lastName" << bsoncxx::builder::stream::open_document
                          << "$regex" << lastName.toStdString()
                          << "$options" << "i" // Case-insensitive search
                          << bsoncxx::builder::stream::close_document;
        }
        if (!phone.isEmpty()) {
            filterBuilder << "phoneNumber" << bsoncxx::builder::stream::open_document
                          << "$regex" << "^" + phone.toStdString() // Matches strings starting with `phone`
                          << bsoncxx::builder::stream::close_document;
        }
        if (!ticket.isEmpty()) {
            filterBuilder << "ticket" << bsoncxx::builder::stream::open_document
                          << "$regex" << ticket.toStdString()
                          << bsoncxx::builder::stream::close_document;
        }

        // Execute the query
        auto cursor = collection.find(filterBuilder.view());
        for (const auto &doc : cursor) {
            QMap<QString, QVariant> data = fromBson(doc);

            // Convert QMap to Customer
            Customer customer(
                data["_id"].toString(),
                data["firstName"].toString(),
                data["lastName"].toString(),
                data["phoneNumber"].toString(),
                data["email"].toString(),
                Address(
                    data["address"].toMap()["street"].toString(),
                    data["address"].toMap()["city"].toString(),
                    data["address"].toMap()["state"].toString(),
                    data["address"].toMap()["zip"].toString()
                ),
                data["note"].toString(),
                data["balance"].toDouble(),
                data["storeCreditBalance"].toDouble()
            );

            customers.append(customer);
        }

        qDebug() << "Found" << customers.size() << "customers matching the criteria.";

    } catch (const mongocxx::exception &e) {
        qDebug() << "Error searching customers:" << e.what();
    }

    return customers;
}

void MongoManager::changeDatabase(const QString &dbName) {
    try {
        this->dbName = dbName;
        database = client[dbName.toStdString()];
        qDebug() << "Switched to MongoDB database:" << dbName;
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error switching database:" << e.what();
    }
}

quint64 MongoManager::getNextId() {
    try {
        auto collection = database["NextId"];

        // Find the nextId document
        auto result = collection.find_one(
            bsoncxx::builder::stream::document{} << "_id" << "nextId" << bsoncxx::builder::stream::finalize
        );

        if (result) {
            auto view = result->view();
            if (view["nextId"].type() == bsoncxx::type::k_int64) {
                return static_cast<quint64>(view["nextId"].get_int64().value);
            }
        } else {
            qDebug() << "NextId document not found. Initializing...";
            setNextId(1); // Initialize the nextId if it doesn't exist
            return 1;
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error getting next ID:" << e.what();
    }

    return 0; // Return 0 if an error occurs
}

bool MongoManager::setNextId(quint64 nextId) {
    try {
        auto collection = database["NextId"];

        // Upsert (insert or update) the nextId document
        auto result = collection.update_one(
            bsoncxx::builder::stream::document{} << "_id" << "nextId" << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$set" << bsoncxx::builder::stream::open_document
                                                 << "nextId" << static_cast<int64_t>(nextId)
                                                 << bsoncxx::builder::stream::close_document
                                                 << bsoncxx::builder::stream::finalize,
            mongocxx::options::update{}.upsert(true) // Ensure the document is created if it doesn't exist
        );

        return result && result->modified_count() > 0 || result->upserted_id();
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error setting next ID:" << e.what();
    }

    return false;
}

quint64 MongoManager::getThenIncrementNextId() {
    try {
        auto collection = database["NextId"];

        // Atomically find and increment the nextId field
        auto result = collection.find_one_and_update(
            bsoncxx::builder::stream::document{} << "_id" << "nextId" << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << "$inc" << bsoncxx::builder::stream::open_document
                                                 << "nextId" << 1 << bsoncxx::builder::stream::close_document
                                                 << bsoncxx::builder::stream::finalize,
            mongocxx::options::find_one_and_update{}.return_document(mongocxx::options::return_document::k_before)
        );

        if (result) {
            auto view = result->view();
            if (view["nextId"].type() == bsoncxx::type::k_int64) {
                return static_cast<quint64>(view["nextId"].get_int64().value);
            }
        } else {
            qDebug() << "NextId document not found. Initializing...";
            setNextId(1); // Initialize the nextId if it doesn't exist
            return 1;
        }
    } catch (const mongocxx::exception &e) {
        qDebug() << "Error getting and incrementing next ID:" << e.what();
    }

    return 0; // Return 0 if an error occurs
}