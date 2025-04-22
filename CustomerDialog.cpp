#include "CustomerDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>

CustomerDialog::CustomerDialog(QWidget *parent, const QMap<QString, QVariant> &customerData)
    : QDialog(parent),
      firstNameEdit(new QLineEdit(this)),
      lastNameEdit(new QLineEdit(this)),
      phoneNumberEdit(new QLineEdit(this)),
      emailEdit(new QLineEdit(this)) {
    // Layout
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->addRow("First Name:", firstNameEdit);
    formLayout->addRow("Last Name:", lastNameEdit);
    formLayout->addRow("Phone Number:", phoneNumberEdit);
    formLayout->addRow("Email:", emailEdit);

    // Populate fields if editing an existing customer
    if (!customerData.isEmpty()) {
        firstNameEdit->setText(customerData["firstName"].toString());
        lastNameEdit->setText(customerData["lastName"].toString());
        phoneNumberEdit->setText(customerData["phoneNumber"].toString());
        emailEdit->setText(customerData["email"].toString());
    }

    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    formLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QMap<QString, QVariant> CustomerDialog::getCustomerData() const {
    return {
        {"firstName", firstNameEdit->text()},
        {"lastName", lastNameEdit->text()},
        {"phoneNumber", phoneNumberEdit->text()},
        {"email", emailEdit->text()}
    };
}