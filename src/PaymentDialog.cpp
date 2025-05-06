#include "PaymentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

PaymentDialog::PaymentDialog(QWidget *parent, double orderTotal)
    : QDialog(parent), orderTotal(orderTotal) {
    setWindowTitle("Payment");
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Order Total Display
    orderTotalLabel = new QLabel(QString("Order Total: $%1").arg(orderTotal, 0, 'f', 2), this);
    orderTotalLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(orderTotalLabel);

    // Payment Amount Input
    QHBoxLayout *amountLayout = new QHBoxLayout();
    QLabel *amountLabel = new QLabel("Payment Amount:", this);
    paymentAmountEdit = new QLineEdit(this);
    paymentAmountEdit->setPlaceholderText("Enter amount...");
    paymentAmountEdit->setText(QString::number(orderTotal, 'f', 2));
    amountLayout->addWidget(amountLabel);
    amountLayout->addWidget(paymentAmountEdit);
    mainLayout->addLayout(amountLayout);

    // Payment Method Selection
    QHBoxLayout *methodLayout = new QHBoxLayout();
    QLabel *methodLabel = new QLabel("Payment Method:", this);
    paymentMethodCombo = new QComboBox(this);
    paymentMethodCombo->addItems({"Cash", "Credit Card", "Check", "Store Credit"});
    methodLayout->addWidget(methodLabel);
    methodLayout->addWidget(paymentMethodCombo);
    mainLayout->addLayout(methodLayout);

    // Check Number Input (initially hidden)
    QHBoxLayout *checkLayout = new QHBoxLayout();
    QLabel *checkLabel = new QLabel("Check Number:", this);
    checkNumberEdit = new QLineEdit(this);
    checkNumberEdit->setPlaceholderText("Enter check number...");
    checkNumberEdit->setVisible(false);
    checkLabel->setVisible(false);
    checkLayout->addWidget(checkLabel);
    checkLayout->addWidget(checkNumberEdit);
    mainLayout->addLayout(checkLayout);

    // Connect payment method change to show/hide check number
    connect(paymentMethodCombo, &QComboBox::currentTextChanged, this, [this, checkLabel](const QString &method) {
        bool isCheck = method == "Check";
        checkNumberEdit->setVisible(isCheck);
        checkLabel->setVisible(isCheck);
    });

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Validate payment amount on OK
    connect(okButton, &QPushButton::clicked, this, [this, orderTotal]() {
        bool ok;
        double amount = paymentAmountEdit->text().toDouble(&ok);
        if (!ok || amount <= 0) {
            QMessageBox::warning(this, "Invalid Amount", "Please enter a valid payment amount greater than 0.");
            paymentAmountEdit->setStyleSheet("background-color: #ffcccc;");
            return;
        }
        if (amount > orderTotal) {
            QMessageBox::warning(this, "Invalid Amount", "Payment amount cannot exceed the remaining balance.");
            paymentAmountEdit->setStyleSheet("background-color: #ffcccc;");
            return;
        }
        paymentAmountEdit->setStyleSheet("");
        QDialog::accept();  // Call QDialog::accept() directly
    });
}

QString PaymentDialog::getSelectedPaymentMethod() const {
    return paymentMethodCombo->currentText();
}

QString PaymentDialog::getCheckNumber() const {
    return checkNumberEdit->text();
}

double PaymentDialog::getPaymentAmount() const {
    return paymentAmountEdit->text().toDouble();
}

void PaymentDialog::setPaymentMethod(const QString &method) {
    int index = paymentMethodCombo->findText(method);
    if (index != -1) {
        paymentMethodCombo->setCurrentIndex(index);
    }
}

void PaymentDialog::setCheckNumber(const QString &number) {
    checkNumberEdit->setText(number);
}

void PaymentDialog::setPaymentAmount(double amount) {
    paymentAmountEdit->setText(QString::number(amount, 'f', 2));
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