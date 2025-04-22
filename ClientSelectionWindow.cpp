#include "ClientSelectionWindow.h"
#include "Store.h"
#include "Customer.h"
#include "CustomerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm> // For std::max

ClientSelectionWindow::ClientSelectionWindow(QWidget *parent)
    : QMainWindow(parent),
      addCustomerButton(new QPushButton("Add Customer", this)),
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
    resultTable->setColumnCount(6); // Number of fields to display
    resultTable->setHorizontalHeaderLabels({"First Name", "Last Name", "Phone", "Ticket", "Order Date", "Balance"});
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make the table read-only
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Allow row selection
    resultTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Layout for input fields and button
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(firstNameEdit);
    inputLayout->addWidget(lastNameEdit);
    inputLayout->addWidget(phoneEdit);
    inputLayout->addWidget(ticketEdit);
    inputLayout->addWidget(searchButton);

    // Add widgets to the main layout
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(resultTable);

    // Create Drop-off and Pick-up buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    dropOffButton = new QPushButton("Drop-off", this);
    pickUpButton = new QPushButton("Pick-up", this);

    dropOffButton->setEnabled(false); // Disabled by default
    pickUpButton->setEnabled(false);  // Disabled by default

    buttonLayout->addWidget(dropOffButton);
    buttonLayout->addWidget(pickUpButton);
    mainLayout->addLayout(buttonLayout);

    // Add Customer and Edit Customer buttons
    mainLayout->addWidget(addCustomerButton);
    mainLayout->addWidget(editCustomerButton);

    setCentralWidget(centralWidget);

    // Set window size
    resize(900, 900);

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

void ClientSelectionWindow::onSearch()
{
    // Get the CSV file path from the Store singleton
    QString filePath = Store::instance().getStoreCsv();

    if (filePath.isEmpty()) {
        qDebug() << "No CSV file path set for the selected store.";
        return;
    }

    // Get input values
    QString firstName = firstNameEdit->text();
    QString lastName = lastNameEdit->text();
    QString phone = phoneEdit->text();
    QString ticket = ticketEdit->text();

    // Perform the search
    searchCsv(filePath, firstName, lastName, phone, ticket);
}

void ClientSelectionWindow::searchCsv(const QString &filePath, const QString &firstName, const QString &lastName, const QString &phone, const QString &ticket)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    QStringList headers = in.readLine().split(','); // Read the header row

    // Get the indices of the required columns
    int firstNameIndex = headers.indexOf("FirstName");
    int lastNameIndex = headers.indexOf("LastName");
    int phoneIndex = headers.indexOf("PhoneNumber");
    int ticketIndex = headers.indexOf("Ticket");
    int orderDateIndex = headers.indexOf("OrderDate");
    int balanceIndex = headers.indexOf("Balance");

    if (firstNameIndex == -1 || lastNameIndex == -1 || phoneIndex == -1 || ticketIndex == -1 || orderDateIndex == -1 || balanceIndex == -1) {
        qDebug() << "Required columns not found in the CSV file.";
        return;
    }

    resultTable->setRowCount(0); // Clear previous results

    // Calculate the maximum index
    int maxIndex = std::max({firstNameIndex, lastNameIndex, phoneIndex, ticketIndex, orderDateIndex, balanceIndex});

    // Use a QMap to store the last entry for each name/phone combination
    QMap<QString, QStringList> uniqueEntries;

    // Search through the file
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList row = line.split(',');

        // Ensure the row has enough entries
        if (row.size() <= maxIndex) {
            qDebug() << "Skipping incomplete row:" << line;
            continue;
        }

        // Check for matches
        bool firstNameMatch = firstName.isEmpty() || row[firstNameIndex].contains(firstName, Qt::CaseInsensitive);
        bool lastNameMatch = lastName.isEmpty() || row[lastNameIndex].contains(lastName, Qt::CaseInsensitive);
        bool phoneMatch = phone.isEmpty() || row[phoneIndex].contains(phone, Qt::CaseInsensitive);
        bool ticketMatch = ticket.isEmpty() || row[ticketIndex].contains(ticket, Qt::CaseInsensitive);

        if (firstNameMatch && lastNameMatch && phoneMatch && ticketMatch) {
            // Use the combination of first name, last name, and phone as the key
            QString key = row[firstNameIndex] + "|" + row[lastNameIndex] + "|" + row[phoneIndex];
            uniqueEntries[key] = row; // Store the latest row for this key
        }
    }

    // Populate the table with the unique entries
    int rowIndex = 0;
    for (const QStringList &row : uniqueEntries) {
        resultTable->insertRow(rowIndex);
        resultTable->setItem(rowIndex, 0, new QTableWidgetItem(row[firstNameIndex]));
        resultTable->setItem(rowIndex, 1, new QTableWidgetItem(row[lastNameIndex]));
        resultTable->setItem(rowIndex, 2, new QTableWidgetItem(row[phoneIndex]));
        resultTable->setItem(rowIndex, 3, new QTableWidgetItem(row[ticketIndex]));
        resultTable->setItem(rowIndex, 4, new QTableWidgetItem(row[orderDateIndex]));
        resultTable->setItem(rowIndex, 5, new QTableWidgetItem(row[balanceIndex]));
        rowIndex++;
    }

    // Resize columns to fit contents
    resultTable->resizeColumnsToContents();

    file.close();
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

        // Update the Customer singleton
        Customer::instance().setName(firstName + " " + lastName);
        Customer::instance().setPhone(phone);

        qDebug() << "Customer updated for Drop-off:"
                 << "Name:" << Customer::instance().getName()
                 << "Phone:" << Customer::instance().getPhone();

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

        // Update the Customer singleton
        Customer::instance().setName(firstName + " " + lastName);
        Customer::instance().setPhone(phone);

        qDebug() << "Customer updated for Pick-up:"
                 << "Name:" << Customer::instance().getName()
                 << "Phone:" << Customer::instance().getPhone();

        // Emit the signal to transition to the PickUpWindow (if applicable)
        emit pickUpRequested();
    }
}

void ClientSelectionWindow::onAddCustomerClicked() {
    CustomerDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Handle adding a new customer
        QMap<QString, QVariant> customerData = dialog.getCustomerData();
        // Add customer to the database (use MongoManager or similar)
        qDebug() << "Adding customer:" << customerData;
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