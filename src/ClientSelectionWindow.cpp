#include "ClientSelectionWindow.h"
#include "Store.h"
#include "Customer.h"
#include "CustomerDialog.h"
#include "Session.h"
#include "MongoManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm> // For std::max
#include <QHeaderView>

ClientSelectionWindow::ClientSelectionWindow(QWidget *parent)
    : QMainWindow(parent),
      addCustomerButton(new QPushButton("New Customer", this)),
      editCustomerButton(new QPushButton("Edit Customer", this))
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Create input fields
    firstNameEdit = new QLineEdit(this);
    firstNameEdit->setPlaceholderText("First Name");
    lastNameEdit = new QLineEdit(this);
    lastNameEdit->setPlaceholderText("Last Name");
    phoneEdit = new QLineEdit(this);
    phoneEdit->setPlaceholderText("Phone");
    ticketEdit = new QLineEdit(this);
    ticketEdit->setPlaceholderText("Ticket");

    // Create search button
    searchButton = new QPushButton("Search", this);

    // Create result table
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(5); // Number of fields to display
    resultTable->setHorizontalHeaderLabels({"First Name", "Last Name", "Phone", "Balance", "Address"});
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make the table read-only
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Allow row selection
    resultTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Set the table to stretch and take up as much space as possible
    resultTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Make columns stretch proportionally
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Layout for input fields and button
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(firstNameEdit);
    inputLayout->addWidget(lastNameEdit);
    inputLayout->addWidget(phoneEdit);
    inputLayout->addWidget(ticketEdit);
    inputLayout->addWidget(searchButton);

    // Add widgets to the main layout
    mainLayout->addLayout(inputLayout, 0);
    mainLayout->addWidget(resultTable, 1); // Add the table to the layout

    // Create Drop-off and Pick-up buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    dropOffButton = new QPushButton("Drop-off", this);
    pickUpButton = new QPushButton("Pick-up", this);

    dropOffButton->setEnabled(false); // Disabled by default
    pickUpButton->setEnabled(false);  // Disabled by default

    // Add Drop-off and Pick-up buttons to the layout
    buttonLayout->addWidget(dropOffButton);
    buttonLayout->addWidget(pickUpButton);

    // Add Add Customer and Edit Customer buttons to the same layout
    buttonLayout->addWidget(addCustomerButton);
    buttonLayout->addWidget(editCustomerButton);

    // Add the button layout to the main layout
    mainLayout->addLayout(buttonLayout, 0);

    mainLayout->addStretch(); // Push content to the top

    setCentralWidget(centralWidget);

    // Set window size
    resize(1280, 1024);

    // Connect search button and textboxes to the search slot
    connect(searchButton, &QPushButton::clicked, this, &ClientSelectionWindow::onSearch);
    connect(firstNameEdit, &QLineEdit::returnPressed, this, &ClientSelectionWindow::onSearch);
    connect(lastNameEdit, &QLineEdit::returnPressed, this, &ClientSelectionWindow::onSearch);
    connect(phoneEdit, &QLineEdit::returnPressed, this, &ClientSelectionWindow::onSearch);
    connect(ticketEdit, &QLineEdit::returnPressed, this, &ClientSelectionWindow::onSearch);

    // Connect row selection to enabling buttons
    connect(resultTable, &QTableWidget::itemSelectionChanged, this, &ClientSelectionWindow::onRowSelected);

    // Connect Drop-off button to its slot
    connect(dropOffButton, &QPushButton::clicked, this, &ClientSelectionWindow::onDropOffClicked);

    // Connect Pick-up button to its slot
    connect(pickUpButton, &QPushButton::clicked, this, &ClientSelectionWindow::onPickUpClicked);

    // Connect Add Customer and Edit Customer buttons to their slots
    connect(addCustomerButton, &QPushButton::clicked, this, &ClientSelectionWindow::onAddCustomerClicked);
    connect(editCustomerButton, &QPushButton::clicked, this, &ClientSelectionWindow::onEditCustomerClicked);
}

void ClientSelectionWindow::onSearch() {
    QString firstName = firstNameEdit->text();
    QString lastName = lastNameEdit->text();
    QString phone = phoneEdit->text();
    QString ticket = ticketEdit->text();

    // Use the MongoManager from the Session singleton
    QList<QMap<QString, QVariant>> customers = Session::instance().getMongoManager().searchCustomers(firstName, lastName, phone, ticket);

    resultTable->setRowCount(0);
    for (const QMap<QString, QVariant>& customer : customers) {
        int row = resultTable->rowCount();
        resultTable->insertRow(row);
        resultTable->setItem(row, 0, new QTableWidgetItem(customer["firstName"].toString()));
        resultTable->setItem(row, 1, new QTableWidgetItem(customer["lastName"].toString()));
        resultTable->setItem(row, 2, new QTableWidgetItem(customer["phoneNumber"].toString()));
        resultTable->setItem(row, 3, new QTableWidgetItem(customer["balance"].toString()));

        QMap<QString, QVariant> address = customer["address"].toMap();
        QString fullAddress = address["street"].toString() + ", " +
                              address["city"].toString() + ", " +
                              address["state"].toString() + " " +
                              address["zip"].toString();

        resultTable->setItem(row, 4, new QTableWidgetItem(fullAddress));
    }

    // Resize columns and rows to fit the content
    resultTable->resizeColumnsToContents();
    //resultTable->resizeRowsToContents();
}

void ClientSelectionWindow::onRowSelected()
{
    // Enable buttons only if a row is selected
    bool hasSelection = resultTable->selectionModel()->hasSelection();
    dropOffButton->setEnabled(hasSelection);
    pickUpButton->setEnabled(hasSelection);
}

// Slot for Drop-off button click
void ClientSelectionWindow::onDropOffClicked()
{
    // Ensure a row is selected
    int selectedRow = resultTable->currentRow();
    if (selectedRow >= 0) {
        // Retrieve customer information from the selected row
        QString firstName = resultTable->item(selectedRow, 0)->text();
        QString lastName = resultTable->item(selectedRow, 1)->text();
        QString phone = resultTable->item(selectedRow, 2)->text();

        // Update the Session singleton
        Session::instance().setCustomer({
            {"firstName", firstName},
            {"lastName", lastName},
            {"phoneNumber", phone}
        });

        qDebug() << "Customer updated for Drop-off:"
                 << "Name:" << Session::instance().getCustomer()["firstName"].toString() + " " + Session::instance().getCustomer()["lastName"].toString()
                 << "Phone:" << Session::instance().getCustomer()["phoneNumber"].toString();

        // Emit the signal to transition to the DropoffWindow
        emit dropOffRequested();
    }
}

// Slot for Pick-up button click
void ClientSelectionWindow::onPickUpClicked()
{
    // Ensure a row is selected
    int selectedRow = resultTable->currentRow();
    if (selectedRow >= 0) {
        // Retrieve customer information from the selected row
        QString firstName = resultTable->item(selectedRow, 0)->text();
        QString lastName = resultTable->item(selectedRow, 1)->text();
        QString phone = resultTable->item(selectedRow, 2)->text();

        // Update the Session singleton
        Session::instance().setCustomer({
            {"firstName", firstName},
            {"lastName", lastName},
            {"phoneNumber", phone}
        });

        qDebug() << "Customer updated for Pick-up:"
                 << "Name:" << Session::instance().getCustomer()["firstName"].toString() + " " + Session::instance().getCustomer()["lastName"].toString()
                 << "Phone:" << Session::instance().getCustomer()["phoneNumber"].toString();

        // Emit the signal to transition to the PickUpWindow (if applicable)
        emit pickUpRequested();
    }
}

void ClientSelectionWindow::onAddCustomerClicked() {
    CustomerDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Get the customer data from the dialog
        QMap<QString, QVariant> customerData = dialog.getCustomerData();

        // Add the customer to the database
        QString customerId = Session::instance().getMongoManager().addCustomer(customerData);

        if (!customerId.isEmpty()) {
            qDebug() << "Customer added successfully with ID:" << customerId;
        } else {
            qDebug() << "Failed to add customer.";
        }
    }
}

void ClientSelectionWindow::onEditCustomerClicked() {
    // Example: Fetch existing customer data (replace with actual logic)
    QMap<QString, QVariant> existingCustomerData = {
        {"firstName", "John"},
        {"lastName", "Doe"},
        {"phoneNumber", "555-1234"},
        {"email", "john.doe@example.com"}
    };

    CustomerDialog dialog(this, existingCustomerData);
    if (dialog.exec() == QDialog::Accepted) {
        // Handle editing the customer
        QMap<QString, QVariant> updatedCustomerData = dialog.getCustomerData();
        // Update customer in the database (use MongoManager or similar)
        qDebug() << "Updating customer:" << updatedCustomerData;
    }
}