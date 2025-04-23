#ifndef STORE_H
#define STORE_H

#include <QObject>
#include <QString>
#include <QMap>

class Store : public QObject {
    Q_OBJECT

public:
    static Store &instance();

    QString getSelectedStore() const;
    QString getStoreCsv() const; // Getter for the CSV file path
    void setSelectedStore(const QString &storeName);

signals:
    void storeUpdated(); // Signal emitted when the selected store is updated

private:
    explicit Store(const QString &storeName = "");

    QString selectedStore;
    QString storeCsv; // Field to store the CSV file path
    QMap<QString, QString> storeToCsvMap; // Map of store names to CSV file paths

    void initializeStoreToCsvMap(); // Method to initialize the map

    // Delete copy constructor and assignment operator
    Store(const Store &) = delete;
    Store &operator=(const Store &) = delete;
};

#endif // STORE_H