#ifndef CUSTOMERDIALOG_H
#define CUSTOMERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
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

    // New fields
    QLineEdit *streetEdit;
    QLineEdit *cityEdit;
    QLineEdit *stateEdit;
    QLineEdit *zipEdit;
    QTextEdit *noteEdit;
    QLineEdit *storeCreditEdit;
};

#endif // CUSTOMERDIALOG_H