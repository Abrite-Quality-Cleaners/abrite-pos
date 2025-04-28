#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>
#include <QMap>
#include <QVariant>
#include "Address.h"

class Customer {
public:
    // Default Constructor
    Customer() : id(""), firstName(""), lastName(""), phoneNumber(""), email(""),
                 address(Address("", "", "", "")), note(""), balance(0.0), storeCreditBalance(0.0) {}

    // Parameterized Constructor
    Customer(const QString &id, const QString &firstName, const QString &lastName,
             const QString &phoneNumber, const QString &email, const Address &address,
             const QString &note, double balance, double storeCreditBalance)
        : id(id), firstName(firstName), lastName(lastName), phoneNumber(phoneNumber),
          email(email), address(address), note(note), balance(balance),
          storeCreditBalance(storeCreditBalance) {}

    QString id;
    QString firstName;
    QString lastName;
    QString phoneNumber;
    QString email;
    Address address;
    QString note;
    double balance;
    double storeCreditBalance;

    QString getFullName() const { return firstName + " " + lastName; }
    void setAddress(const Address &addr) { address = addr; }
    Address getAddress() const { return address; }
};

#endif // CUSTOMER_H