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
#include <QSizePolicy>

PickupWindow::PickupWindow(QWidget *parent)
    : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    setCentralWidget(central);
    resize(1280, 1024); // Updated to match DropoffWindow size
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
    customerOrdersTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Enable full row selection
    customerOrdersTable->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(customerOrdersTable, &QTableWidget::itemSelectionChanged, this, &PickupWindow::onOrderSelected);

    leftLayout->addWidget(ordersLabel);
    leftLayout->addWidget(customerOrdersTable);

    mainLayout->addLayout(leftLayout, 2); // Left side occupies 2/3 of the window

    // Right-side layout (Receipt widget, Total, and Order Notes)
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Order ID Label
    orderIdLabel = new QLabel(this);
    orderIdLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    orderIdLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(orderIdLabel);

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

    // Payment Method Section
    QHBoxLayout *paymentMethodRow = new QHBoxLayout();
    QLabel *paymentMethodLabel = new QLabel("Payment Method:", this);
    paymentMethodLabel->setStyleSheet("font-weight: bold;");
    paymentMethodEdit = new QLineEdit("On-pickup", this);
    paymentMethodEdit->setReadOnly(true);
    paymentMethodEdit->setFixedWidth(100);

    QLabel *amountPaidLabel = new QLabel("Amount Paid:", this);
    amountPaidLabel->setStyleSheet("font-weight: bold;");
    amountPaidEdit = new QLineEdit("$0.00", this);
    amountPaidEdit->setReadOnly(true);
    amountPaidEdit->setFixedWidth(100);

    paymentMethodRow->addWidget(paymentMethodLabel);
    paymentMethodRow->addWidget(paymentMethodEdit);
    paymentMethodRow->addWidget(amountPaidLabel);
    paymentMethodRow->addWidget(amountPaidEdit);
    paymentMethodRow->addStretch();
    rightLayout->addLayout(paymentMethodRow);

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
    connect(paymentButton, &QPushButton::clicked, this, &PickupWindow::handlePayment);

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

        // Store the order ID in the first column's item
        dropoffDateItem->setData(Qt::UserRole, order["_id"].toString());

        customerOrdersTable->setItem(row, 0, dropoffDateItem);
        customerOrdersTable->setItem(row, 1, readyDateItem);
        customerOrdersTable->setItem(row, 2, paymentTypeItem);
        customerOrdersTable->setItem(row, 3, orderTotalItem);
    }
}

void PickupWindow::onOrderSelected() {
    // Clear the receipt table
    receiptTable->setRowCount(0);

    // Get the selected row
    int selectedRow = customerOrdersTable->currentRow();
    if (selectedRow < 0) {
        qDebug() << "No order selected.";
        orderIdLabel->setText("");
        totalLabel->setText("Total: $0.00");
        return;
    }

    // Get the order ID from the selected row
    QString orderId = customerOrdersTable->item(selectedRow, 0)->data(Qt::UserRole).toString();
    if (orderId.isEmpty()) {
        qDebug() << "No order ID found for selected row.";
        orderIdLabel->setText("");
        totalLabel->setText("Total: $0.00");
        return;
    }

    // Get the order data directly using the ID
    QMap<QString, QVariant> selectedOrder = Session::instance().getMongoManager().getOrder(orderId);
    if (selectedOrder.isEmpty()) {
        qDebug() << "Selected order not found.";
        orderIdLabel->setText("");
        totalLabel->setText("Total: $0.00");
        return;
    }

    // Update Order ID label
    orderIdLabel->setText(QString("Order #%1").arg(orderId));

    // Update total label
    double orderTotal = selectedOrder["orderTotal"].toDouble();
    totalLabel->setText(QString("Total: $%1").arg(orderTotal, 0, 'f', 2));

    // Update payment method display
    QString paymentType = selectedOrder["paymentType"].toString();
    if (!paymentType.isEmpty()) {
        if (paymentType == "Check") {
            // Extract check number from order notes
            QString notes = selectedOrder["orderNote"].toString();
            int checkIndex = notes.indexOf("Check #: ");
            if (checkIndex != -1) {
                QString checkNumber = notes.mid(checkIndex + 9).split("\n")[0];
                paymentMethodEdit->setText(QString("Check #%1").arg(checkNumber));
            } else {
                paymentMethodEdit->setText("Check");
            }
        } else {
            paymentMethodEdit->setText(paymentType);
        }
        double amountPaid = orderTotal - selectedOrder["balance"].toDouble();
        amountPaidEdit->setText(QString("$%1").arg(amountPaid, 0, 'f', 2));
    } else {
        paymentMethodEdit->setText("On-pickup");
        amountPaidEdit->setText("$0.00");
    }

    // Check if the subOrders field is empty
    QVariantList subOrders = selectedOrder["subOrders"].toList();
    if (subOrders.isEmpty()) {
        // Add a header row for legacy orders
        int row = receiptTable->rowCount();
        receiptTable->insertRow(row);
        QTableWidgetItem *legacyHeader = new QTableWidgetItem("Legacy order, items unknown...");
        legacyHeader->setFlags(Qt::NoItemFlags); // Make it non-editable
        legacyHeader->setTextAlignment(Qt::AlignCenter);
        receiptTable->setSpan(row, 0, 1, receiptTable->columnCount()); // Span across all columns
        receiptTable->setItem(row, 0, legacyHeader);
        return;
    }

    // Populate the receipt table with items and categories
    for (const QVariant &subOrderVariant : subOrders) {
        QMap<QString, QVariant> type = subOrderVariant.toMap();

        // Add a header row for the type with ID in brackets
        int headerRow = receiptTable->rowCount();
        receiptTable->insertRow(headerRow);
        QString headerText = QString("%1 [%2]").arg(type["type"].toString()).arg(type["id"].toInt());
        QTableWidgetItem *headerItem = new QTableWidgetItem(headerText);
        headerItem->setFlags(Qt::NoItemFlags); // Make it non-editable
        headerItem->setTextAlignment(Qt::AlignCenter);
        receiptTable->setSpan(headerRow, 0, 1, receiptTable->columnCount()); // Span across all columns
        receiptTable->setItem(headerRow, 0, headerItem);

        // Add the items in the type
        QVariantList items = type["items"].toList();
        for (const QVariant &itemVariant : items) {
            QMap<QString, QVariant> item = itemVariant.toMap();

            int itemRow = receiptTable->rowCount();
            receiptTable->insertRow(itemRow);

            QTableWidgetItem *itemName = new QTableWidgetItem(item["name"].toString());
            QTableWidgetItem *itemPrice = new QTableWidgetItem(QString::number(item["price"].toDouble(), 'f', 2));
            QTableWidgetItem *itemQuantity = new QTableWidgetItem(QString::number(item["quantity"].toInt()));

            receiptTable->setItem(itemRow, 0, itemName);
            receiptTable->setItem(itemRow, 1, itemPrice);
            receiptTable->setItem(itemRow, 2, itemQuantity);
        }
    }
}

void PickupWindow::handlePayment() {
    // Get the selected row
    int selectedRow = customerOrdersTable->currentRow();
    if (selectedRow < 0) {
        qDebug() << "No order selected.";
        return;
    }

    // Get the order ID from the selected row
    QString orderId = customerOrdersTable->item(selectedRow, 0)->data(Qt::UserRole).toString();
    if (orderId.isEmpty()) {
        qDebug() << "No order ID found for selected row.";
        return;
    }

    // Get the order data
    QMap<QString, QVariant> selectedOrder = Session::instance().getMongoManager().getOrder(orderId);
    if (selectedOrder.isEmpty()) {
        qDebug() << "Selected order not found.";
        return;
    }

    // Show payment dialog with order total
    double orderTotal = selectedOrder["orderTotal"].toDouble();
    PaymentDialog paymentDialog(this, orderTotal);

    // Set existing payment information if available
    QString existingPaymentMethod = selectedOrder["paymentType"].toString();
    if (!existingPaymentMethod.isEmpty()) {
        paymentDialog.setPaymentMethod(existingPaymentMethod);
        double existingAmount = orderTotal - selectedOrder["balance"].toDouble();
        paymentDialog.setPaymentAmount(existingAmount);
        if (existingPaymentMethod == "Check") {
            // Extract check number from order notes
            QString notes = selectedOrder["orderNote"].toString();
            int checkIndex = notes.indexOf("Check #: ");
            if (checkIndex != -1) {
                QString checkNumber = notes.mid(checkIndex + 9).split("\n")[0];
                paymentDialog.setCheckNumber(checkNumber);
            }
        }
    }

    if (paymentDialog.exec() == QDialog::Accepted) {
        QString paymentMethod = paymentDialog.getSelectedPaymentMethod();
        QString checkNumber = paymentDialog.getCheckNumber();
        double paymentAmount = paymentDialog.getPaymentAmount();

        // Calculate new balance
        double currentBalance = selectedOrder["balance"].toDouble();
        double newBalance = currentBalance - paymentAmount;

        // Update order with payment information
        QMap<QString, QVariant> updateData;
        updateData["paymentType"] = paymentMethod;
        updateData["paymentDate"] = QDateTime::currentDateTime().toString("MM/dd/yy hh:mm:ss");
        updateData["paymentEmployee"] = Session::instance().getUser().getUsername();
        updateData["balance"] = newBalance;  // Ensure balance is included in the update

        if (paymentMethod == "Check") {
            // Add check number to order notes
            QString notes = selectedOrder["orderNote"].toString();
            if (!notes.isEmpty()) {
                notes += "\n";
            }
            notes += "Check #: " + checkNumber;
            updateData["orderNote"] = notes;
        }

        // Update the order in the database
        if (Session::instance().getMongoManager().updateOrder(orderId, updateData)) {
            // Update the payment method display
            if (paymentMethod == "Check") {
                paymentMethodEdit->setText(QString("Check #%1").arg(checkNumber));
            } else {
                paymentMethodEdit->setText(paymentMethod);
            }
            amountPaidEdit->setText(QString("$%1").arg(paymentAmount, 0, 'f', 2));
            
            // Refresh the orders table
            populateOrdersTable();
            
            // Select the same order again to refresh the display
            customerOrdersTable->selectRow(selectedRow);
        } else {
            qDebug() << "Failed to update order with payment information.";
        }
    }
}