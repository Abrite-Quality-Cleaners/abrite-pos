#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>
#include <QMap>
#include <QVariant>
#include "Address.h"

class Customer {
public:
    QString id;
    QString firstName;
    QString lastName;
    QString phoneNumber;
    QString email;
    Address address;
    QString note;
    double balance;
    double storeCreditBalance;

    // Getters and setters
    QString getFullName() const { return firstName + " " + lastName; }
    void setAddress(const Address &addr) { address = addr; }
    Address getAddress() const { return address; }
};

#endif // CUSTOMER_H