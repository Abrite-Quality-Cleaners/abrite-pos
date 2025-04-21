#include "Customer.h"
#include <QDebug>

// Singleton instance
Customer &Customer::instance() {
    static Customer instance;
    return instance;
}

Customer::Customer(const QString &name, const QString &phone)
    : QObject(nullptr), name(name), phone(phone) {}

QString Customer::getName() const {
    return name;
}

QString Customer::getPhone() const {
    return phone;
}

void Customer::setName(const QString &name) {
    this->name = name;
    qDebug() << "Customer name set to:" << name;
    emit customerUpdated(); // Emit signal
}

void Customer::setPhone(const QString &phone) {
    this->phone = phone;
    qDebug() << "Customer phone set to:" << phone;
    emit customerUpdated(); // Emit signal
}