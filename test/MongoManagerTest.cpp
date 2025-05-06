#include "MongoManager.h"
#include <gtest/gtest.h>
#include "Customer.h"
#include "Address.h"
#include "Order.h"

class MongoManagerTest : public ::testing::Test {
protected:
    static MongoManager *mongoManager;  // Static MongoManager for the test suite

    // Per-test setup
    void SetUp() override {
        // Clean up the test database before each test
        mongoManager->getDatabase()["Customers"].delete_many({});
        mongoManager->getDatabase()["Orders"].delete_many({});
        mongoManager->getDatabase()["NextId"].delete_many({});
    }

    // Per-test teardown
    void TearDown() override {
    }

    // Suite-wide setup
    static void SetUpTestSuite() {
        mongoManager = new MongoManager("mongodb://localhost:27017", "abrite-pos-test");
    }

    // Suite-wide teardown
    static void TearDownTestSuite() {
        delete mongoManager; // Destroy the MongoManager
    }
};

// Static member definitions
MongoManager *MongoManagerTest::mongoManager = nullptr;

TEST_F(MongoManagerTest, AddAndGetCustomer) {
    QMap<QString, QVariant> customerData = {
        {"firstName", "John"},
        {"lastName", "Doe"},
        {"address", QMap<QString, QVariant>{
            {"street", "123 Main St"},
            {"city", "Springfield"},
            {"state", "IL"},
            {"zip", "62704"}
        }},
        {"phoneNumber", "555-1234"},
        {"email", "john.doe@example.com"},
        {"note", "Preferred customer"},
        {"balance", 50.0},
        {"storeCreditBalance", 20.0}
    };

    QString customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    QMap<QString, QVariant> fetchedCustomer = mongoManager->getCustomer(customerId);
    ASSERT_EQ(fetchedCustomer["firstName"].toString(), "John");
    ASSERT_EQ(fetchedCustomer["lastName"].toString(), "Doe");

    // Dump the contents of the "customers" collection
    mongoManager->dumpCollection("customers");
}

TEST_F(MongoManagerTest, AddAndGetOrder) {
    QMap<QString, QVariant> customerData = {
        {"firstName", "John"},
        {"lastName", "Doe"}
    };
    QString customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    QMap<QString, QVariant> orderData = {
        {"customerId", customerId},
        {"store", "Store A"},
        {"subOrders", QVariantList{
            QMap<QString, QVariant>{
                {"type", "Dryclean"},
                {"items", QVariantList{
                    QMap<QString, QVariant>{{"name", "Pants"}, {"price", 10.0}, {"quantity", 2}},
                    QMap<QString, QVariant>{{"name", "Jacket"}, {"price", 15.0}, {"quantity", 1}}
                }},
                {"total", 35.0}
            }
        }},
        {"orderTotal", 35.0},
        {"balance", 35.0},  // Initial balance equals order total
        {"status", "in-progress"}
    };

    QString orderId = mongoManager->addOrder(orderData);
    ASSERT_FALSE(orderId.isEmpty());

    QMap<QString, QVariant> fetchedOrder = mongoManager->getOrder(orderId);
    ASSERT_EQ(fetchedOrder["store"].toString(), "Store A");
    ASSERT_EQ(fetchedOrder["orderTotal"].toDouble(), 35.0);
    ASSERT_EQ(fetchedOrder["balance"].toDouble(), 35.0);

    // Dump the contents of the "orders" collection
    mongoManager->dumpCollection("orders");
}

TEST_F(MongoManagerTest, AddAndRetrieveCustomerObject) {
    // Create a Customer object using the constructor
    Customer customer(
        "", // ID will be assigned by MongoDB
        "John", // First Name
        "Doe", // Last Name
        "555-1234", // Phone Number
        "john.doe@example.com", // Email
        Address("123 Main St", "Springfield", "IL", "62704"), // Address
        "Preferred customer", // Note
        50.0, // Balance
        20.0 // Store Credit Balance
    );

    // Add the customer to the database
    QString customerId = mongoManager->addCustomer(customer);
    ASSERT_FALSE(customerId.isEmpty());

    // Retrieve the customer from the database
    Customer fetchedCustomer = mongoManager->getCustomerById(customerId);

    // Verify the retrieved customer matches the original
    ASSERT_EQ(fetchedCustomer.firstName, "John");
    ASSERT_EQ(fetchedCustomer.lastName, "Doe");
    ASSERT_EQ(fetchedCustomer.phoneNumber, "555-1234");
    ASSERT_EQ(fetchedCustomer.email, "john.doe@example.com");
    ASSERT_EQ(fetchedCustomer.address.street, "123 Main St");
    ASSERT_EQ(fetchedCustomer.address.city, "Springfield");
    ASSERT_EQ(fetchedCustomer.address.state, "IL");
    ASSERT_EQ(fetchedCustomer.address.zip, "62704");
    ASSERT_EQ(fetchedCustomer.note, "Preferred customer");
    ASSERT_EQ(fetchedCustomer.balance, 50.0);
    ASSERT_EQ(fetchedCustomer.storeCreditBalance, 20.0);
}

TEST_F(MongoManagerTest, AddAndRetrieveOrderObject) {
    Order order;
    order.customerId = "64a7b2f5e4b0c123456789ab";
    order.store = "Abrite Deliveries";
    order.subOrders = {
        {0, "Dryclean", {{"Pants", 10.0, 2}, {"Jacket", 15.0, 1}}, 35.0}
    };
    order.orderTotal = 35.0;
    order.balance = 35.0;  // Initial balance equals order total
    order.status = "in-progress";
    order.ticketNumber = "T12345";
    order.dropoffDate = "2023-10-01";
    order.dropoffEmployee = "John";
    order.pickupDate = "2023-10-05";

    QString orderId = mongoManager->addOrder(order);
    ASSERT_FALSE(orderId.isEmpty());

    Order fetchedOrder = mongoManager->getOrderById(orderId);
    ASSERT_EQ(fetchedOrder.customerId, "64a7b2f5e4b0c123456789ab");
    ASSERT_EQ(fetchedOrder.store, "Abrite Deliveries");
    ASSERT_EQ(fetchedOrder.orderTotal, 35.0);
    ASSERT_EQ(fetchedOrder.balance, 35.0);
    ASSERT_EQ(fetchedOrder.subOrders.size(), 1);
    ASSERT_EQ(fetchedOrder.subOrders[0].type, "Dryclean");
    ASSERT_EQ(fetchedOrder.subOrders[0].items.size(), 2);
    ASSERT_EQ(fetchedOrder.subOrders[0].items[0].name, "Pants");
    ASSERT_EQ(fetchedOrder.subOrders[0].items[0].price, 10.0);
    ASSERT_EQ(fetchedOrder.subOrders[0].items[0].quantity, 2);
}

TEST_F(MongoManagerTest, SearchCustomers) {
    QMap<QString, QVariant> customerData = {
        {"firstName", "John"},
        {"lastName", "Doe"},
        {"phoneNumber", "555-1234"},
        {"ticket", "T12345"}
    };
    QString customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    customerData = {
        {"firstName", "John"},
        {"lastName", "DEF"},
        {"address", QMap<QString, QVariant>{
            {"street", "123 Main St"},
            {"city", "Springfield"},
            {"state", "IL"},
            {"zip", "62704"}
        }},
        {"phoneNumber", "555-2389"},
        {"email", "john.doe@example.com"},
        {"note", "Preferred customer"},
        {"balance", 50.0},
        {"storeCreditBalance", 20.0}
    };
    customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    customerData = {
        {"firstName", "Test"},
        {"lastName", "User"},
        {"phoneNumber", "555-5555"},
        {"ticket", "9823546"}
    };
    customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    QList<Customer> results = mongoManager->searchCustomers("John", "Doe", "555-1234", "T12345");
    ASSERT_EQ(results.size(), 1);
    ASSERT_EQ(results[0].firstName, "John");
    ASSERT_EQ(results[0].lastName, "Doe");
    ASSERT_EQ(results[0].phoneNumber, "555-1234");
    //ASSERT_EQ(results[0].ticket, "T12345");

    qDebug() << "Search results:";
    for (const auto &result : results) {
        qDebug() << "Customer:" << result.firstName << result.lastName;
    }

    results = mongoManager->searchCustomers("John", "", "", "");
    ASSERT_EQ(results.size(), 2);

    qDebug() << "Search results:";
    for (const auto &result : results) {
        qDebug() << "Customer:" << result.firstName << result.lastName;
    }

    results = mongoManager->searchCustomers("", "", "555", "");
    ASSERT_EQ(results.size(), 3);

    qDebug() << "Search results:";
    for (const auto &result : results) {
        qDebug() << "Customer:" << result.firstName << result.lastName;
    }
}

TEST_F(MongoManagerTest, AddOrderWithCorrectCustomerId) {
    QMap<QString, QVariant> customerData = {
        {"firstName", "John"},
        {"lastName", "Doe"}
    };
    QString customerId = mongoManager->addCustomer(customerData);
    qDebug() << "Adding order for customer ID:" << customerId;
    ASSERT_FALSE(customerId.isEmpty());

    QMap<QString, QVariant> orderData = {
        {"customerId", customerId},
        {"store", "Store A"},
        {"subOrders", QVariantList{}},
        {"orderTotal", 0.0},
        {"status", "in-progress"}
    };

    QString orderId = mongoManager->addOrder(orderData);
    qDebug() << "Order added with ID:" << orderId;
    ASSERT_FALSE(orderId.isEmpty());

    QMap<QString, QVariant> fetchedOrder = mongoManager->getOrder(orderId);
    qDebug() << "Customer ID of fetched order:" << fetchedOrder["customerId"].toString();
    ASSERT_EQ(fetchedOrder["customerId"].toString(), customerId);
}

TEST_F(MongoManagerTest, DeserializeCustomerId) {
    QMap<QString, QVariant> orderData = {
        {"customerId", "64a7b2f5e4b0c123456789ab"},
        {"store", "Store A"},
        {"subOrders", QVariantList{}},
        {"orderTotal", 0.0},
        {"status", "in-progress"}
    };

    QString orderId = mongoManager->addOrder(orderData);
    ASSERT_FALSE(orderId.isEmpty());

    QMap<QString, QVariant> fetchedOrder = mongoManager->getOrder(orderId);
    ASSERT_EQ(fetchedOrder["customerId"].toString(), "64a7b2f5e4b0c123456789ab");
}

TEST_F(MongoManagerTest, InitializeNextIdIfNotExists) {
    // Drop the NextId collection to simulate a missing document
    mongoManager->getDatabase()["NextId"].drop();

    // Initialize the document
    quint64 initialId = 1000;
    bool result = mongoManager->setNextId(initialId);
    ASSERT_TRUE(result);

    // Call getNextId, which should initialize the document
    quint64 nextId = mongoManager->getNextId();
    ASSERT_EQ(nextId, initialId); // Should initialize to 1

    // Verify that the document was created with the initial ID
    quint64 nextId2 = mongoManager->getNextId();
    ASSERT_EQ(nextId2, initialId);
}

TEST_F(MongoManagerTest, SetAndGetNextIdWithoutIncrement) {
    // Set the next ID to a specific value
    quint64 initialId = 2000;
    ASSERT_TRUE(mongoManager->setNextId(initialId));

    // Verify that getNextId retrieves the current ID without incrementing
    quint64 currentId = mongoManager->getNextId();
    ASSERT_EQ(currentId, initialId);

    // Verify that the ID remains unchanged
    currentId = mongoManager->getNextId();
    ASSERT_EQ(currentId, initialId);
}

TEST_F(MongoManagerTest, SetAndGetThenIncrementNextId) {
    // Set the next ID to a specific value
    quint64 initialId = 3000;
    ASSERT_TRUE(mongoManager->setNextId(initialId));

    // Verify that getThenIncrementNextId retrieves the ID
    quint64 nextId = mongoManager->getThenIncrementNextId();
    ASSERT_EQ(nextId, initialId);

    // Verify that the next call retrieves an incremented ID
    nextId = mongoManager->getThenIncrementNextId();
    ASSERT_EQ(nextId, initialId + 1);
}

TEST_F(MongoManagerTest, AddAndRetrieveOrderWithSubOrderId) {
    Order order;
    order.customerId = "64a7b2f5e4b0c123456789ab";
    order.store = "Abrite Deliveries";
    order.subOrders = {
        {1001, "Dryclean", {{"Pants", 10.0, 2}, {"Jacket", 15.0, 1}}, 35.0},
        {1002, "Laundry", {{"Towel", 5.0, 3}}, 15.0}
    };
    order.orderTotal = 50.0;
    order.status = "in-progress";

    QString orderId = mongoManager->addOrder(order);
    ASSERT_FALSE(orderId.isEmpty());

    Order fetchedOrder = mongoManager->getOrderById(orderId);
    ASSERT_EQ(fetchedOrder.subOrders.size(), 2);
    ASSERT_EQ(fetchedOrder.subOrders[0].id, 1001);
    ASSERT_EQ(fetchedOrder.subOrders[1].id, 1002);
}

// Add new test for payment and balance updates
TEST_F(MongoManagerTest, UpdateOrderPaymentAndBalance) {
    // First create a customer
    QMap<QString, QVariant> customerData = {
        {"firstName", "John"},
        {"lastName", "Doe"}
    };
    QString customerId = mongoManager->addCustomer(customerData);
    ASSERT_FALSE(customerId.isEmpty());

    // Create an order
    QMap<QString, QVariant> orderData = {
        {"customerId", customerId},
        {"store", "Store A"},
        {"subOrders", QVariantList{
            QMap<QString, QVariant>{
                {"type", "Dryclean"},
                {"items", QVariantList{
                    QMap<QString, QVariant>{{"name", "Pants"}, {"price", 10.0}, {"quantity", 2}},
                    QMap<QString, QVariant>{{"name", "Jacket"}, {"price", 15.0}, {"quantity", 1}}
                }},
                {"total", 35.0}
            }
        }},
        {"orderTotal", 35.0},
        {"balance", 35.0},  // Initial balance equals order total
        {"status", "in-progress"}
    };

    QString orderId = mongoManager->addOrder(orderData);
    ASSERT_FALSE(orderId.isEmpty());

    // Update the order with a payment
    QMap<QString, QVariant> updateData = {
        {"paymentType", "Cash"},
        {"paymentDate", "2023-10-01 14:30:00"},
        {"paymentEmployee", "John"},
        {"balance", 20.0}  // $15 payment made
    };

    bool updateSuccess = mongoManager->updateOrder(orderId, updateData);
    ASSERT_TRUE(updateSuccess);

    // Verify the update
    QMap<QString, QVariant> updatedOrder = mongoManager->getOrder(orderId);
    ASSERT_EQ(updatedOrder["paymentType"].toString(), "Cash");
    ASSERT_EQ(updatedOrder["balance"].toDouble(), 20.0);

    // Test check payment with check number
    updateData = {
        {"paymentType", "Check"},
        {"paymentDate", "2023-10-01 15:30:00"},
        {"paymentEmployee", "John"},
        {"balance", 0.0},  // Full payment
        {"orderNote", "Check #: 12345"}
    };

    updateSuccess = mongoManager->updateOrder(orderId, updateData);
    ASSERT_TRUE(updateSuccess);

    // Verify the check payment update
    updatedOrder = mongoManager->getOrder(orderId);
    ASSERT_EQ(updatedOrder["paymentType"].toString(), "Check");
    ASSERT_EQ(updatedOrder["balance"].toDouble(), 0.0);
    ASSERT_TRUE(updatedOrder["orderNote"].toString().contains("Check #: 12345"));
}