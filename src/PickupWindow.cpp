#include "PickupWindow.h"
#include "Session.h"
#include "MongoManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QSplitter>
#include <QDateTime>
#include "PaymentDialog.h"

PickupWindow::PickupWindow(QWidget *parent)
    : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    setCentralWidget(central);
    resize(900, 900);
    setWindowTitle("Pickup Interface");

    // Left-side layout (Orders table, Customer info, and Customer notes)
    QVBoxLayout *leftLayout = new QVBoxLayout();

    // Customer Info Section
    QHBoxLayout *customerInfoLayout = new QHBoxLayout();
    QLabel *customerLabel = new QLabel("Customer:", this);
    customerLabel->setStyleSheet("font-weight: bold;");
    customerNameEdit = new QLineEdit(this);
    customerNameEdit->setFixedWidth(400);
    customerNameEdit->setReadOnly(true);
    customerNameEdit->setPlaceholderText("No customer selected");

    customerInfoLayout->addWidget(customerLabel);
    customerInfoLayout->addWidget(customerNameEdit);
    customerInfoLayout->addStretch();
    leftLayout->addLayout(customerInfoLayout);

    // Customer Notes Section
    QLabel *customerNotesLabel = new QLabel("Customer Notes:", this);
    customerNotesLabel->setStyleSheet("font-weight: bold;");
    customerNotesEdit = new QTextEdit(this);
    customerNotesEdit->setReadOnly(true);
    customerNotesEdit->setFixedHeight(100);

    leftLayout->addWidget(customerNotesLabel);
    leftLayout->addWidget(customerNotesEdit);

    // Orders Table Section
    QLabel *ordersLabel = new QLabel("Customer Orders:", this);
    ordersLabel->setStyleSheet("font-weight: bold;");
    customerOrdersTable = new QTableWidget(this);
    customerOrdersTable->setColumnCount(4);
    customerOrdersTable->setHorizontalHeaderLabels({"Dropoff Date", "Ready Date", "Payment Type", "Order Total"});
    customerOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    customerOrdersTable->verticalHeader()->setVisible(false);

    leftLayout->addWidget(ordersLabel);
    leftLayout->addWidget(customerOrdersTable);

    mainLayout->addLayout(leftLayout, 2); // Left side occupies 2/3 of the window

    // Right-side layout (Receipt widget, Total, and Order Notes)
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Receipt Table
    receiptTable = new QTableWidget(this);
    receiptTable->setColumnCount(3);
    receiptTable->setHorizontalHeaderLabels({"Item", "Price", "Quantity"});
    receiptTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receiptTable->setSelectionMode(QAbstractItemView::NoSelection);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    receiptTable->verticalHeader()->setVisible(false);
    rightLayout->addWidget(receiptTable);

    // Total Label
    totalLabel = new QLabel("Total: $0.00", this);
    totalLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    rightLayout->addWidget(totalLabel);

    // Order Notes Section
    QLabel *orderNotesLabel = new QLabel("Order Notes:", this);
    orderNotesLabel->setStyleSheet("font-weight: bold;");
    notesEdit = new QTextEdit(this);
    notesEdit->setPlaceholderText("Enter notes about the order...");
    notesEdit->setFixedHeight(100);
    rightLayout->addWidget(orderNotesLabel);
    rightLayout->addWidget(notesEdit);

    // Buttons Row
    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *checkoutButton = new QPushButton("Check-out", this);
    checkoutButton->setMinimumWidth(100);
    btnRow->addWidget(checkoutButton);
    connect(checkoutButton, &QPushButton::clicked, this, &PickupWindow::handleCheckout);

    QPushButton *paymentButton = new QPushButton("Payment", this);
    paymentButton->setMinimumWidth(100);
    btnRow->addWidget(paymentButton);
    connect(paymentButton, &QPushButton::clicked, this, [=]() {
        PaymentDialog paymentDialog(this);
        if (paymentDialog.exec() == QDialog::Accepted) {
            QString paymentMethod = paymentDialog.getSelectedPaymentMethod();
            QString checkNumber = paymentDialog.getCheckNumber();

            qDebug() << "Payment method selected:" << paymentMethod;
            if (paymentMethod == "Check") {
                qDebug() << "Check Number:" << checkNumber;
            }
        }
    });

    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumWidth(100);
    btnRow->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        emit pickupDone(); // Emit the pickupDone signal
    });

    rightLayout->addLayout(btnRow);

    mainLayout->addLayout(rightLayout, 1); // Right side occupies 1/3 of the window
}

void PickupWindow::handleCheckout() {
    qDebug() << "Check-out button clicked";
    qDebug() << "Order Notes:" << notesEdit->toPlainText();
    // Add logic for handling checkout here
}

void PickupWindow::updateCustomerInfo() {
    const Customer &customer = Session::instance().getCustomer();
    QString customerName = customer.firstName + " " + customer.lastName;
    QString customerId = customer.id;

    qDebug() << "Updating customer info for Pickup:"
             << "Name:" << customerName
             << "ID:" << customerId;

    if (!customerName.trimmed().isEmpty() && !customerId.isEmpty()) {
        customerNameEdit->setText(customerName + " (" + customerId + ")");
        customerNotesEdit->setText(customer.note); // Display customer notes
    } else {
        customerNameEdit->setText("No customer selected");
        customerNotesEdit->clear();
    }

    populateOrdersTable();
}

void PickupWindow::populateOrdersTable() {
    // Clear the table
    customerOrdersTable->setRowCount(0);

    // Get the customer from the session
    const Customer &customer = Session::instance().getCustomer();
    QString customerId = customer.id;

    if (customerId.isEmpty()) {
        qDebug() << "No customer selected.";
        return;
    }

    // Fetch orders for the customer
    QList<QMap<QString, QVariant>> orders = Session::instance().getMongoManager().getOrdersByCustomer(customerId);

    qDebug() << "Fetched" << orders.size() << "orders for customer ID:" << customerId;

    // Sort orders by dropoff date (most recent to oldest)
    std::sort(orders.begin(), orders.end(), [](const QMap<QString, QVariant> &a, const QMap<QString, QVariant> &b) {
        QDateTime dropoffA = QDateTime::fromString(a["dropoffDate"].toString(), "MM/dd/yy hh:mm:ss");
        QDateTime dropoffB = QDateTime::fromString(b["dropoffDate"].toString(), "MM/dd/yy hh:mm:ss");
        return dropoffA > dropoffB; // Most recent first
    });

    // Populate the table
    for (const QMap<QString, QVariant> &order : orders) {
        int row = customerOrdersTable->rowCount();
        customerOrdersTable->insertRow(row);

        QTableWidgetItem *dropoffDateItem = new QTableWidgetItem(order["dropoffDate"].toString());
        QTableWidgetItem *readyDateItem = new QTableWidgetItem(order["orderReadyDate"].toString());
        QTableWidgetItem *paymentTypeItem = new QTableWidgetItem(order["paymentType"].toString());
        QTableWidgetItem *orderTotalItem = new QTableWidgetItem(QString::number(order["orderTotal"].toDouble(), 'f', 2));

        customerOrdersTable->setItem(row, 0, dropoffDateItem);
        customerOrdersTable->setItem(row, 1, readyDateItem);
        customerOrdersTable->setItem(row, 2, paymentTypeItem);
        customerOrdersTable->setItem(row, 3, orderTotalItem);
    }
}