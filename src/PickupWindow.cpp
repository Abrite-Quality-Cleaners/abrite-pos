#include "PickupWindow.h"
#include "Session.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QSplitter>
#include "PaymentDialog.h"

PickupWindow::PickupWindow(QWidget *parent)
    : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    setCentralWidget(central);
    resize(900, 900);
    setWindowTitle("Pickup Interface");

    // Top bar for Ticket ID and Customer fields
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    ticketIdDisplay = new QLineEdit(this);
    ticketIdDisplay->setReadOnly(true);
    ticketIdDisplay->setFixedWidth(100);
    customerNameEdit = new QLineEdit(this);
    customerNameEdit->setFixedWidth(200);
    customerNameEdit->setPlaceholderText("Enter customer name");

    topBarLayout->addWidget(new QLabel("Ticket ID:"));
    topBarLayout->addWidget(ticketIdDisplay);
    topBarLayout->addSpacing(15);
    topBarLayout->addWidget(new QLabel("Customer:"));
    topBarLayout->addWidget(customerNameEdit);
    topBarLayout->addStretch();

    mainLayout->addLayout(topBarLayout); // Add top bar above both layouts

    QHBoxLayout *contentLayout = new QHBoxLayout();

    // Left layout for customer notes and orders
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);

    // Customer notes text box
    QTextEdit *customerNotes = new QTextEdit(this);
    customerNotes->setPlaceholderText("Enter customer notes...");
    leftSplitter->addWidget(customerNotes);

    // Customer orders table
    QTableWidget *customerOrdersTable = new QTableWidget(this);
    customerOrdersTable->setColumnCount(3);
    customerOrdersTable->setHorizontalHeaderLabels({"Order ID", "Item", "Quantity"});
    customerOrdersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    customerOrdersTable->verticalHeader()->setVisible(false);
    leftSplitter->addWidget(customerOrdersTable);

    leftSplitter->setStretchFactor(0, 1); // Customer notes take 1/5th
    leftSplitter->setStretchFactor(1, 4); // Orders table takes 4/5th

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel("Customer Notes:"));
    leftLayout->addWidget(customerNotes);

    leftLayout->addWidget(new QLabel("Orders:"));
    leftLayout->addWidget(customerOrdersTable);

    // Add left layout to content layout
    contentLayout->addLayout(leftLayout, 1); // 50% width

    QVBoxLayout *masterLayout = new QVBoxLayout();

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

    QPushButton *paymentButton = new QPushButton("Payment", this);
    paymentButton->setMinimumWidth(100);
    btnRow->insertWidget(1, paymentButton); // Add between checkout and cancel buttons
    connect(paymentButton, &QPushButton::clicked, this, [=]() {
        PaymentDialog paymentDialog(this);
        if (paymentDialog.exec() == QDialog::Accepted) {
            QString paymentMethod = paymentDialog.getSelectedPaymentMethod();
            QString checkNumber = paymentDialog.getCheckNumber();

            qDebug() << "Payment method selected:" << paymentMethod;
            if (paymentMethod == "Check") {
                qDebug() << "Check Number:" << checkNumber;
            }

            // Handle the selected payment method here
        }
    });

    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumWidth(100);
    btnRow->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        emit pickupDone(); // Emit the pickupDone signal
    });

    masterLayout->addLayout(btnRow);

    // Add existing layout to content layout
    contentLayout->addLayout(masterLayout, 1); // 50% width

    mainLayout->addLayout(contentLayout); // Add content layout below top bar

    // Initialize the customer info
    updateCustomerInfo();
}

void PickupWindow::handleCheckout() {
    qDebug() << "Check-out button clicked";
    qDebug() << "Order Notes:" << notesEdit->toPlainText();
    // Add logic for handling checkout here
}

void PickupWindow::updateCustomerInfo() {
    QString customerName = Session::instance().getCustomer()["firstName"].toString() + " " + Session::instance().getCustomer()["lastName"].toString();
    QString customerPhone = Session::instance().getCustomer()["phoneNumber"].toString();

    if (!customerName.isEmpty()) {
        customerNameEdit->setText(customerName + " (" + customerPhone + ")");
    } else {
        customerNameEdit->setText("No customer selected");
    }
}