#include "Store.h"
#include <QDebug>

// Singleton instance
Store &Store::instance() {
    static Store instance;
    return instance;
}

Store::Store(const QString &storeName)
    : QObject(nullptr), selectedStore(storeName)
{
    initializeStoreToCsvMap();
}

void Store::initializeStoreToCsvMap() {
    // Map store names to their corresponding CSV file paths
    storeToCsvMap["Abrite Deliveries"] = "/home/keith/data/combined.abrite-deliveries.csv";
    storeToCsvMap["Sparkle"] = "/home/keith/data/combined.sparkle.csv";
}

QString Store::getSelectedStore() const {
    return selectedStore;
}

QString Store::getStoreCsv() const {
    return storeCsv;
}

void Store::setSelectedStore(const QString &storeName) {
    selectedStore = storeName;

    // Update the storeCsv field based on the map
    if (storeToCsvMap.contains(storeName)) {
        storeCsv = storeToCsvMap[storeName];
    } else {
        storeCsv.clear(); // Clear the CSV path if the store is not in the map
    }

    qDebug() << "Selected store set to:" << storeName;
    qDebug() << "CSV path set to:" << storeCsv;

    emit storeUpdated(); // Emit signal
}