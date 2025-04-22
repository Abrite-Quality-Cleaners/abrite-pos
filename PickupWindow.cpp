#include "PickupWindow.h"
#include "Customer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

PickupWindow::PickupWindow(QWidget *parent)
    : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    QVBoxLayout *masterLayout = new QVBoxLayout(central);

    // Top bar
    QHBoxLayout *topRow = new QHBoxLayout();
    ticketIdDisplay = new QLineEdit(this);
    ticketIdDisplay->setReadOnly(true);
    ticketIdDisplay->setFixedWidth(100);
    customerNameEdit = new QLineEdit(this);
    customerNameEdit->setFixedWidth(200);
    customerNameEdit->setPlaceholderText("Enter customer name");

    topRow->addWidget(new QLabel("Ticket ID:"));
    topRow->addWidget(ticketIdDisplay);
    topRow->addSpacing(15);
    topRow->addWidget(new QLabel("Customer:"));
    topRow->addWidget(customerNameEdit);
    topRow->addStretch();
    masterLayout->addLayout(topRow);

    // Receipt table
    receiptTable = new QTableWidget(this);
    receiptTable->setColumnCount(3);
    receiptTable->setHorizontalHeaderLabels({"Item", "Price", "Quantity"});
    receiptTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receiptTable->setSelectionMode(QAbstractItemView::NoSelection);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    receiptTable->verticalHeader()->setVisible(false);
    masterLayout->addWidget(receiptTable);

    // Total label
    totalLabel = new QLabel("Total: $0.00", this);
    totalLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    masterLayout->addWidget(totalLabel);

    // Notes textbox
    notesEdit = new QTextEdit(this);
    notesEdit->setPlaceholderText("Enter notes about the order...");
    notesEdit->setFixedHeight(100);
    masterLayout->addWidget(notesEdit);

    // Buttons
    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *checkoutButton = new QPushButton("Check-out", this);
    checkoutButton->setMinimumWidth(100);
    btnRow->addWidget(checkoutButton);
    connect(checkoutButton, &QPushButton::clicked, this, &PickupWindow::handleCheckout);

    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumWidth(100);
    btnRow->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        emit pickupDone(); // Emit the pickupDone signal
    });

    masterLayout->addLayout(btnRow);

    setCentralWidget(central);
    resize(900, 900);
    setWindowTitle("Pickup Interface");

    // Initialize the customer info
    updateCustomerInfo();
}

void PickupWindow::handleCheckout() {
    qDebug() << "Check-out button clicked";
    qDebug() << "Order Notes:" << notesEdit->toPlainText();
    // Add logic for handling checkout here
}

void PickupWindow::updateCustomerInfo() {
    QString customerName = Customer::instance().getName();
    QString customerPhone = Customer::instance().getPhone();

    if (!customerName.isEmpty()) {
        customerNameEdit->setText(customerName + " (" + customerPhone + ")");
    } else {
        customerNameEdit->setText("No customer selected");
    }
}