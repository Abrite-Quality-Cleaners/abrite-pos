#ifndef ORDER_H
#define ORDER_H

#include <QMap>
#include <QVariant>
#include <QList>
#include <QString>

struct OrderItem {
    QString name;
    double price;
    int quantity;
};

struct OrderCategory {
    QString category;
    QList<OrderItem> items;
    double categoryTotal;
};

struct Order {
    QString id;  // MongoDB _id as a string (ObjectId -> hex string)
    QString customerId;  // MongoDB _id as a string
    QString store;  // "Abrite Deliveries"
    QList<OrderCategory> orderItems;  // Empty list for now
    double orderTotal;
    QString status;  // "legacy"
    QString ticketNumber;
    QString dropoffDate;
    QString dropoffEmployee;
    QString pickupDate;
    QString pickupEmployee;
    QString paymentDate;
    QString paymentType;
    QString paymentEmployee;
    QVariant voidDate;   // Nullable (use QVariant())
    QVariant voidEmployee;  // Nullable
    QString orderNote;
    QString rackNumber;
    QString orderReadyDate;  // Optional field; may not be set
};

#endif // ORDER_H