#ifndef CUSTOMERDIALOG_H
#define CUSTOMERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QMap>
#include <QVariant>

class CustomerDialog : public QDialog {
    Q_OBJECT

public:
    explicit CustomerDialog(QWidget *parent = nullptr, const QMap<QString, QVariant> &customerData = {});
    QMap<QString, QVariant> getCustomerData() const;

private:
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *phoneNumberEdit;
    QLineEdit *emailEdit;
};

#endif // CUSTOMERDIALOG_H