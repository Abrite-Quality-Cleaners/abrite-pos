#include "MongoManager.h"
#include <gtest/gtest.h>

class MongoManagerTest : public ::testing::Test {
protected:
    static MongoManager *mongoManager;  // Static MongoManager for the test suite

    // Per-test setup
    void SetUp() override {
        // Clean up the test database before each test
        mongoManager->getDatabase()["customers"].delete_many({});
        mongoManager->getDatabase()["orders"].delete_many({});
    }

    // Per-test teardown
    void TearDown() override {
        // Clean up the test database after each test
        mongoManager->getDatabase()["customers"].delete_many({});
        mongoManager->getDatabase()["orders"].delete_many({});
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