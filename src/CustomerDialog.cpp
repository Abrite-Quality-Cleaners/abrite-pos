#include "CustomerDialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>

CustomerDialog::CustomerDialog(QWidget *parent, const QMap<QString, QVariant> &customerData)
    : QDialog(parent),
      firstNameEdit(new QLineEdit(this)),
      lastNameEdit(new QLineEdit(this)),
      phoneNumberEdit(new QLineEdit(this)),
      emailEdit(new QLineEdit(this)),
      streetEdit(new QLineEdit(this)),
      cityEdit(new QLineEdit(this)),
      stateEdit(new QLineEdit(this)),
      zipEdit(new QLineEdit(this)),
      noteEdit(new QTextEdit(this)),
      storeCreditEdit(new QLineEdit(this)) {
    // Layout
    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->addRow("First Name:", firstNameEdit);
    formLayout->addRow("Last Name:", lastNameEdit);
    formLayout->addRow("Phone Number:", phoneNumberEdit);
    formLayout->addRow("Email:", emailEdit);

    // Address fields
    formLayout->addRow("Street:", streetEdit);
    formLayout->addRow("City:", cityEdit);
    formLayout->addRow("State:", stateEdit);
    formLayout->addRow("Zip Code:", zipEdit);

    // Note field
    noteEdit->setPlaceholderText("Enter a note about the customer...");
    formLayout->addRow("Note:", noteEdit);

    // Store credit field
    storeCreditEdit->setPlaceholderText("Enter store credit balance...");
    formLayout->addRow("Store Credit:", storeCreditEdit);

    // Populate fields if editing an existing customer
    if (!customerData.isEmpty()) {
        firstNameEdit->setText(customerData["firstName"].toString());
        lastNameEdit->setText(customerData["lastName"].toString());
        phoneNumberEdit->setText(customerData["phoneNumber"].toString());
        emailEdit->setText(customerData["email"].toString());

        QMap<QString, QVariant> address = customerData["address"].toMap();
        streetEdit->setText(address["street"].toString());
        cityEdit->setText(address["city"].toString());
        stateEdit->setText(address["state"].toString());
        zipEdit->setText(address["zip"].toString());

        noteEdit->setText(customerData["note"].toString());
        storeCreditEdit->setText(QString::number(customerData["storeCreditBalance"].toDouble()));
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
        {"email", emailEdit->text()},
        {"address", QMap<QString, QVariant>{
            {"street", streetEdit->text()},
            {"city", cityEdit->text()},
            {"state", stateEdit->text()},
            {"zip", zipEdit->text()}
        }},
        {"note", noteEdit->toPlainText()},
        {"storeCreditBalance", storeCreditEdit->text().toDouble()}
    };
}