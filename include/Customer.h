#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QObject>
#include <QString>

class Customer : public QObject {
    Q_OBJECT

public:
    static Customer &instance();

    QString getName() const;
    QString getPhone() const;

    void setName(const QString &name);
    void setPhone(const QString &phone);

signals:
    void customerUpdated(); // Signal emitted when the customer is updated

private:
    explicit Customer(const QString &name = "", const QString &phone = "");

    QString name;
    QString phone;

    // Delete copy constructor and assignment operator
    Customer(const Customer &) = delete;
    Customer &operator=(const Customer &) = delete;
};

#endif // CUSTOMER_H