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
    }

    // Per-test teardown
    void TearDown() override {
        // Clean up the test database after each test
        mongoManager->getDatabase()["Customers"].delete_many({});
        mongoManager->getDatabase()["Orders"].delete_many({});
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
        {"orderItems", QVariantList{
            QMap<QString, QVariant>{
                {"category", "Dryclean"},
                {"items", QVariantList{
                    QMap<QString, QVariant>{{"name", "Pants"}, {"price", 10.0}, {"quantity", 2}},
                    QMap<QString, QVariant>{{"name", "Jacket"}, {"price", 15.0}, {"quantity", 1}}
                }},
                {"categoryTotal", 35.0}
            }
        }},
        {"orderTotal", 35.0},
        {"status", "in-progress"}
    };

    QString orderId = mongoManager->addOrder(orderData);
    ASSERT_FALSE(orderId.isEmpty());

    QMap<QString, QVariant> fetchedOrder = mongoManager->getOrder(orderId);
    ASSERT_EQ(fetchedOrder["store"].toString(), "Store A");
    ASSERT_EQ(fetchedOrder["orderTotal"].toDouble(), 35.0);

    // Dump the contents of the "orders" collection
    mongoManager->dumpCollection("orders");

    // Optionally, dump the entire database
    mongoManager->dumpDatabase();
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
    order.customerId = "customer123";
    order.store = "Abrite Deliveries";
    order.orderItems = {
        {"Dryclean", {{"Pants", 10.0, 2}, {"Jacket", 15.0, 1}}, 35.0}
    };
    order.orderTotal = 35.0;
    order.status = "in-progress";
    order.ticketNumber = "T12345";
    order.dropoffDate = "2023-10-01";
    order.dropoffEmployee = "John";
    order.pickupDate = "2023-10-05";

    QString orderId = mongoManager->addOrder(order);
    ASSERT_FALSE(orderId.isEmpty());

    Order fetchedOrder = mongoManager->getOrderById(orderId);
    ASSERT_EQ(fetchedOrder.customerId, "customer123");
    ASSERT_EQ(fetchedOrder.store, "Abrite Deliveries");
    ASSERT_EQ(fetchedOrder.orderTotal, 35.0);
    ASSERT_EQ(fetchedOrder.orderItems.size(), 1);
    ASSERT_EQ(fetchedOrder.orderItems[0].category, "Dryclean");
    ASSERT_EQ(fetchedOrder.orderItems[0].items.size(), 2);
    ASSERT_EQ(fetchedOrder.orderItems[0].items[0].name, "Pants");
    ASSERT_EQ(fetchedOrder.orderItems[0].items[0].price, 10.0);
    ASSERT_EQ(fetchedOrder.orderItems[0].items[0].quantity, 2);
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