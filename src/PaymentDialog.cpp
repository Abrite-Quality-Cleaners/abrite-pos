#include "PaymentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

PaymentDialog::PaymentDialog(QWidget *parent)
    : QDialog(parent), selectedPaymentMethod(""), checkNumber("") {
    setWindowTitle("Select Payment Method");

    // Create a horizontal layout for the buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // Cash button
    cashButton = new QPushButton("Cash", this);
    cashButton->setMinimumSize(100, 82);
    buttonLayout->addWidget(cashButton);
    connect(cashButton, &QPushButton::clicked, this, &PaymentDialog::handleCash);

    // Credit button
    creditButton = new QPushButton("Credit", this);
    creditButton->setMinimumSize(100, 82);
    buttonLayout->addWidget(creditButton);
    connect(creditButton, &QPushButton::clicked, this, &PaymentDialog::handleCredit);

    // Check button with line edit above it
    QVBoxLayout *checkLayout = new QVBoxLayout();
    checkNumberEdit = new QLineEdit(this);
    checkNumberEdit->setPlaceholderText("Check Number");
    checkNumberEdit->setFixedWidth(100);
    checkButton = new QPushButton("Check", this);
    checkButton->setMinimumSize(100, 50); // Adjust height to match combined size
    checkLayout->addWidget(checkNumberEdit);
    checkLayout->addWidget(checkButton);
    buttonLayout->addLayout(checkLayout);
    connect(checkButton, &QPushButton::clicked, this, &PaymentDialog::handleCheck);

    // Store Credit button
    storeCreditButton = new QPushButton("Store Credit", this);
    storeCreditButton->setMinimumSize(100, 82);
    buttonLayout->addWidget(storeCreditButton);
    connect(storeCreditButton, &QPushButton::clicked, this, &PaymentDialog::handleStoreCredit);

    // Set the layout for the dialog
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addLayout(buttonLayout);
}

QString PaymentDialog::getSelectedPaymentMethod() const {
    return selectedPaymentMethod;
}

QString PaymentDialog::getCheckNumber() const {
    return checkNumber;
}

void PaymentDialog::handleCash() {
    selectedPaymentMethod = "Cash";
    accept();
}

void PaymentDialog::handleCredit() {
    selectedPaymentMethod = "Credit";
    accept();
}

void PaymentDialog::handleCheck() {
    if (checkNumberEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a check number.");
        return;
    }
    selectedPaymentMethod = "Check";
    checkNumber = checkNumberEdit->text();
    accept();
}

void PaymentDialog::handleStoreCredit() {
    selectedPaymentMethod = "Store Credit";
    accept();
}