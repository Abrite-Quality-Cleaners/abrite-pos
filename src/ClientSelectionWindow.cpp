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

    // Set larger font size for text boxes
    QString textBoxStyle = "QLineEdit { font-size: 20px; }";
    firstNameEdit->setStyleSheet(textBoxStyle);
    lastNameEdit->setStyleSheet(textBoxStyle);
    phoneEdit->setStyleSheet(textBoxStyle);
    ticketEdit->setStyleSheet(textBoxStyle);

    // Create search button
    searchButton = new QPushButton("Search", this);

    // Create result table
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(6); // Number of fields to display
    resultTable->setHorizontalHeaderLabels({"First Name", "Last Name", "Phone", "Balance", "Address", "ID"});
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

    // Set larger font size for buttons
    QString buttonStyle = "QPushButton { font-size: 20px; }";
    searchButton->setStyleSheet(buttonStyle);
    dropOffButton->setStyleSheet(buttonStyle);
    pickUpButton->setStyleSheet(buttonStyle);
    addCustomerButton->setStyleSheet(buttonStyle);
    editCustomerButton->setStyleSheet(buttonStyle);

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

    customers = Session::instance().getMongoManager().searchCustomers(firstName, lastName, phone, ticket);

    resultTable->setRowCount(0);
    for (const Customer &customer : customers) {
        int row = resultTable->rowCount();
        resultTable->insertRow(row);

        resultTable->setItem(row, 0, new QTableWidgetItem(customer.firstName));
        resultTable->setItem(row, 1, new QTableWidgetItem(customer.lastName));
        resultTable->setItem(row, 2, new QTableWidgetItem(customer.phoneNumber));
        resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(customer.balance)));

        QString fullAddress = customer.address.street + ", " +
                              customer.address.city + ", " +
                              customer.address.state + " " +
                              customer.address.zip;
        resultTable->setItem(row, 4, new QTableWidgetItem(fullAddress));

        resultTable->setItem(row, 5, new QTableWidgetItem(customer.id));
    }

    resultTable->resizeColumnsToContents();
    resultTable->resizeRowsToContents();
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
        // Retrieve the customer ID from the selected row
        QString customerId = resultTable->item(selectedRow, 5)->text(); // Assuming ID is in column 5

        // Search for the customer in the customers list
        auto it = std::find_if(customers.begin(), customers.end(), [&customerId](const Customer &customer) {
            return customer.id == customerId;
        });

        if (it != customers.end()) {
            // Set the found customer in the Session singleton
            Session::instance().setCustomer(*it);

            qDebug() << "Customer updated for Drop-off:"
                     << "Name:" << it->firstName + " " + it->lastName
                     << "Phone:" << it->phoneNumber;

            // Emit the signal to transition to the DropoffWindow
            emit dropOffRequested();
        } else {
            qDebug() << "Customer with ID" << customerId << "not found in the customers list.";
        }
    }
}

// Slot for Pick-up button click
void ClientSelectionWindow::onPickUpClicked()
{
    // Ensure a row is selected
    int selectedRow = resultTable->currentRow();
    if (selectedRow >= 0) {
        // Retrieve the customer ID from the selected row
        QString customerId = resultTable->item(selectedRow, 5)->text(); // Assuming ID is in column 5

        // Search for the customer in the customers list
        auto it = std::find_if(customers.begin(), customers.end(), [&customerId](const Customer &customer) {
            return customer.id == customerId;
        });

        if (it != customers.end()) {
            // Set the found customer in the Session singleton
            Session::instance().setCustomer(*it);

            qDebug() << "Customer updated for Pick-up:"
                     << "Name:" << it->firstName + " " + it->lastName
                     << "Phone:" << it->phoneNumber;

            // Emit the signal to transition to the PickUpWindow
            emit pickUpRequested();
        } else {
            qDebug() << "Customer with ID" << customerId << "not found in the customers list.";
        }
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
    // Ensure a row is selected
    int selectedRow = resultTable->currentRow();
    if (selectedRow < 0) {
        qDebug() << "No customer selected for editing.";
        return;
    }

    // Retrieve the customer ID or unique identifier from the selected row
    QString customerId = resultTable->item(selectedRow, 5)->text(); // Assuming ID is stored in UserRole
    if (customerId.isEmpty()) {
        qDebug() << "Customer ID not found for the selected row.";
        return;
    }

    // Fetch the existing customer data from the database
    QMap<QString, QVariant> existingCustomerData = Session::instance().getMongoManager().getCustomer(customerId);

    if (existingCustomerData.isEmpty()) {
        qDebug() << "Failed to fetch customer data for ID:" << customerId;
        return;
    }

    // Open the dialog with the existing customer data
    CustomerDialog dialog(this, existingCustomerData);
    if (dialog.exec() == QDialog::Accepted) {
        // Get the updated customer data from the dialog
        QMap<QString, QVariant> updatedCustomerData = dialog.getCustomerData();

        // Update the customer in the database
        bool success = Session::instance().getMongoManager().updateCustomer(customerId, updatedCustomerData);

        if (success) {
            qDebug() << "Customer updated successfully for ID:" << customerId;
        } else {
            qDebug() << "Failed to update customer for ID:" << customerId;
        }
    }
}