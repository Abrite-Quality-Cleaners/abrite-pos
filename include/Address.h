#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>

class Address {
public:
    QString street;
    QString city;
    QString state;
    QString zip;

    Address() = default;
    Address(const QString &street, const QString &city, const QString &state, const QString &zip)
        : street(street), city(city), state(state), zip(zip) {}
};

#endif // ADDRESS_H