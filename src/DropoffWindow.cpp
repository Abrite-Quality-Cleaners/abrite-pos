#include "DropoffWindow.h"
#include "Customer.h"
#include "PaymentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QStyle>
#include <QTimer>
#include "Session.h"

DropoffWindow::DropoffWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *masterLayout = new QVBoxLayout(central);

    // Top bar
    QHBoxLayout *topRow = new QHBoxLayout();
    customerNameEdit = new QLineEdit(this);
    customerNameEdit->setFixedWidth(500);
    customerNameEdit->setReadOnly(true);
    customerNameEdit->setPlaceholderText("No customer selected");
    dateTimeDisplay = new QLineEdit(this);
    dateTimeDisplay->setReadOnly(true);
    dateTimeDisplay->setFixedWidth(200);

    topRow->addWidget(new QLabel("Customer:"));
    topRow->addWidget(customerNameEdit);
    topRow->addSpacing(15);
    topRow->addWidget(new QLabel("Date:"));
    topRow->addWidget(dateTimeDisplay);
    topRow->addStretch();
    masterLayout->addLayout(topRow);

    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout();
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget, 2);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    receiptTable = new QTableWidget(this);
    receiptTable->setColumnCount(4);
    receiptTable->setHorizontalHeaderLabels({"Item", "Price", "Quantity", ""});
    receiptTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receiptTable->setSelectionMode(QAbstractItemView::NoSelection);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    receiptTable->verticalHeader()->setVisible(false);
    receiptTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    receiptTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rightLayout->addWidget(receiptTable);

    totalLabel = new QLabel("Total: $0.00", this);
    totalLabel->setStyleSheet("font-size: 18px; font-weight: bold;"); // Bold and increase font size
    rightLayout->addWidget(totalLabel);

    // Add notes textbox
    notesEdit = new QTextEdit(this);
    notesEdit->setPlaceholderText("Enter notes about the order...");
    notesEdit->setFixedHeight(100); // Set a fixed height for the notes textbox
    rightLayout->addWidget(notesEdit);

    QHBoxLayout *btnRow = new QHBoxLayout();

    // Create "Check-out" button
    QPushButton *checkoutButton = new QPushButton("Check-out", this);
    checkoutButton->setMinimumWidth(100);
    btnRow->addWidget(checkoutButton);
    connect(checkoutButton, &QPushButton::clicked, this, [=]() {
        qDebug() << "Check-out button clicked";
        qDebug() << "Order Notes:" << notesEdit->toPlainText(); // Log the notes
        handleCheckout(); // Call the checkout handler
    });

    // Create "Pay" button
    QPushButton *payButton = new QPushButton("Pay", this);
    payButton->setMinimumWidth(100);
    btnRow->addWidget(payButton);
    connect(payButton, &QPushButton::clicked, this, [=]() {
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

    // Create "Cancel" button
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumWidth(100);
    btnRow->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        qDebug() << "Cancel button clicked";
        emit dropoffDone(); // Emit the dropoffDone signal
    });

    rightLayout->addLayout(btnRow);

    mainLayout->addLayout(rightLayout, 1);
    masterLayout->addLayout(mainLayout);

    setCentralWidget(central);
    resize(1280, 1024);
    setWindowTitle("Dropoff Interface");

    loadPricesFromIni("../prices.ini");

    // Initialize the date and time display
    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &DropoffWindow::updateDateTime);
    dateTimeTimer->start(1000); // Update every second
    updateDateTime(); // Initial update
}

DropoffWindow::~DropoffWindow()
{
    //closePrinter();
}

void DropoffWindow::loadTicketId(const QString &storeIniPath)
{
    QFile f(ticketFile);
    if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        ticketId = in.readLine().toInt();
    } else {
        QSettings s(storeIniPath, QSettings::IniFormat);
        ticketId = s.value("TicketIdRange/min", 1000).toInt();
    }
}

void DropoffWindow::updateTicketIdDisplay()
{
    ticketIdDisplay->setText(QString::number(ticketId));
}

void DropoffWindow::handleCheckout()
{
    QFile f(ticketFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << ticketId + 1; // Save the next ticket ID
    }

    QString client = customerNameEdit->text();
    if (client.isEmpty()) client = "Unknown";

    QString receipt = QString(
        "             Sparkle Cleaners\n"
        "            165 Oak Grove Ave.\n"
        "           Fall River, MA 02720\n\n"
        "CLIENT: %1\n"
        "PHONE : (508) 123 4567\n"
        "DROP  : %2\n"
        "PICKUP: %3\n\n"
        "PAYMENT  : On-pickup\n"
        "EMPLOYEE : NA\n\n"
    ).arg(client,
          QDateTime::currentDateTime().toString("ddd MM/dd/yyyy hh:mm AP"),
          QDateTime::currentDateTime().addDays(3).toString("ddd MM/dd/yyyy hh:mm AP"));

    double total = 0.0;
    int currentTicketId = ticketId; // Start with the current ticket ID

    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        QTableWidgetItem *itemCell = receiptTable->item(row, 0);

        // Check if this is a header row (non-editable)
        if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
            // Add a new section header with the ticket ID
            receipt += QString(
                "------------------------------------------\n"
                "TICKET ID: %1 (%2)\n"
                "------------------------------------------\n"
                "|GARMENT              |QUANTITY  |PRICE  |\n"
                "------------------------------------------\n"
            ).arg(currentTicketId).arg(itemCell->text());
            currentTicketId++; // Increment the ticket ID for the next section
            continue;
        }

        // Process item rows
        QString item = itemCell->text();
        double price = receiptTable->item(row, 1)->text().toDouble();
        int qty = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2))->value();
        double subtotal = qty * price;
        total += subtotal;

        receipt += QString("%1%2%3\n")
            .arg(item.leftJustified(24))
            .arg(QString::number(qty).rightJustified(10))
            .arg(QString::number(subtotal, 'f', 2).rightJustified(10));
    }

    receipt += QString(
        "                       -------------------\n"
        "                       TOTAL:     $%1\n\n"
        "NOTE: %2\n"
    ).arg(total, 0, 'f', 2).arg(notesEdit->toPlainText());

    receipt += "\n\n\n\n\n\n\n"; // Add some blank lines so the cut doesn't chop off part of the receipt

    ticketId = currentTicketId; // Update the ticket ID for the next transaction
    updateTicketIdDisplay();

    qDebug() << "Receipt generated:\n";
    printf("%s\n", receipt.toStdString().c_str());
}

void DropoffWindow::loadPricesFromIni(const QString &filename)
{
    QSettings s(filename, QSettings::IniFormat);
    for (const QString &cat : s.childGroups()) {
        s.beginGroup(cat);
        QList<QPair<QString, double>> items;
        for (const QString &key : s.childKeys()) {
            bool ok;
            double price = s.value(key).toDouble(&ok);
            if (ok) items.append({key, price});
        }
        // Pass the category name (tab name) to createCategoryTab
        tabWidget->addTab(createCategoryTab(cat, items), cat);
        s.endGroup();
    }
}

QWidget *DropoffWindow::createCategoryTab(const QString &categoryName, const QList<QPair<QString, double>> &items)
{
    QWidget *tab = new QWidget(this);
    QGridLayout *grid = new QGridLayout(tab);

    for (int i = 0; i < items.size(); ++i) {
        const QString &itemName = items[i].first;
        double price = items[i].second;

        // Create a button for the item
        QPushButton *btn = new QPushButton(QString("%1\n$%2").arg(itemName).arg(price, 0, 'f', 2), this);

        // Connect the button's clicked signal to addItemToReceipt with the category name
        connect(btn, &QPushButton::clicked, this, [=]() {
            addItemToReceipt(categoryName, itemName, price);
        });

        // Add the button to the grid layout (4 widgets per row)
        grid->addWidget(btn, i / 4, i % 4);
    }

    return tab;
}

void DropoffWindow::addItemToReceipt(const QString &tabName, const QString &itemName, double price)
{
    // Check if the header for this tab has already been added
    int headerRow = -1;
    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        QTableWidgetItem *itemCell = receiptTable->item(row, 0);
        if (itemCell && !itemCell->flags().testFlag(Qt::ItemIsEditable) && itemCell->text() == tabName) {
            headerRow = row;
            break;
        }
    }

    if (headerRow == -1) {
        // Add a header row for the tab if it doesn't exist
        headerRow = receiptTable->rowCount();
        receiptTable->insertRow(headerRow);

        QTableWidgetItem *headerItem = new QTableWidgetItem(tabName);
        headerItem->setFlags(Qt::NoItemFlags); // Make the header row non-editable
        headerItem->setTextAlignment(Qt::AlignCenter);
        headerItem->setBackground(Qt::lightGray); // Highlight the header row
        receiptTable->setSpan(headerRow, 0, 1, receiptTable->columnCount()); // Span across all columns
        receiptTable->setItem(headerRow, 0, headerItem);
    }

    // Check if the item already exists in the receipt table under the correct header
    for (int row = headerRow + 1; row < receiptTable->rowCount(); ++row) {
        QTableWidgetItem *itemCell = receiptTable->item(row, 0);
        if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
            break; // Reached the next header or end of table
        }
        if (itemCell->text() == itemName) {
            // Increment the quantity if the item already exists
            QSpinBox *quantitySpinBox = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2));
            if (quantitySpinBox) {
                quantitySpinBox->setValue(quantitySpinBox->value() + 1);
                updateTotal();
            }
            return; // Exit the method since the item is already handled
        }
    }

    // Add the item row under the correct header
    int itemRow = headerRow + 1;
    while (itemRow < receiptTable->rowCount()) {
        QTableWidgetItem *itemCell = receiptTable->item(itemRow, 0);
        if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
            break; // Reached the next header or end of table
        }
        ++itemRow;
    }

    receiptTable->insertRow(itemRow);
    receiptTable->setItem(itemRow, 0, new QTableWidgetItem(itemName));
    receiptTable->setItem(itemRow, 1, new QTableWidgetItem(QString::number(price, 'f', 2)));

    // Add a quantity spinbox
    QSpinBox *quantitySpinBox = new QSpinBox(this);
    quantitySpinBox->setRange(1, 999);
    quantitySpinBox->setValue(1);
    receiptTable->setCellWidget(itemRow, 2, quantitySpinBox);

    // Connect the spinbox's valueChanged signal to updateTotal
    connect(quantitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DropoffWindow::updateTotal);

    // Add a remove button
    QPushButton *removeButton = new QPushButton("Remove", this);
    receiptTable->setCellWidget(itemRow, 3, removeButton);
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        // Dynamically determine the row of the button
        for (int row = 0; row < receiptTable->rowCount(); ++row) {
            if (receiptTable->cellWidget(row, 3) == removeButton) {
                receiptTable->removeRow(row);
                updateTotal(); // Update the total after removing the row

                // Check if the section header should be removed
                int headerRow = row - 1; // Assume the header is directly above the removed item
                if (headerRow >= 0) {
                    QTableWidgetItem *headerItem = receiptTable->item(headerRow, 0);
                    if (headerItem && !headerItem->flags().testFlag(Qt::ItemIsEditable)) {
                        // Check if there are any remaining items in this section
                        bool hasItems = false;
                        for (int i = headerRow + 1; i < receiptTable->rowCount(); ++i) {
                            QTableWidgetItem *itemCell = receiptTable->item(i, 0);
                            if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
                                break; // Reached the next header or end of table
                            }
                            hasItems = true;
                            break;
                        }

                        if (!hasItems) {
                            // Remove the header row
                            QString tabName = headerItem->text();
                            receiptTable->removeRow(headerRow);
                            addedHeaders.remove(tabName); // Remove the tab name from the addedHeaders set
                        }
                    }
                }

                break; // Exit the loop once the row is found and removed
            }
        }
    });

    // Optionally, update the total here
    updateTotal();
}

void DropoffWindow::removeItem(int row)
{
    QString name = receiptTable->item(row, 0)->text();
    itemRowMap.remove(name);
    receiptTable->removeRow(row);

    // Check if the section header should be removed
    int headerRow = row - 1; // Assume the header is directly above the removed item
    if (headerRow >= 0) {
        QTableWidgetItem *headerItem = receiptTable->item(headerRow, 0);
        if (headerItem && !headerItem->flags().testFlag(Qt::ItemIsEditable)) {
            // Check if there are any remaining items in this section
            bool hasItems = false;
            for (int i = headerRow + 1; i < receiptTable->rowCount(); ++i) {
                QTableWidgetItem *itemCell = receiptTable->item(i, 0);
                if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
                    break; // Reached the next header or end of table
                }
                hasItems = true;
                break;
            }

            if (!hasItems) {
                // Remove the header row
                QString tabName = headerItem->text();
                receiptTable->removeRow(headerRow);
                addedHeaders.remove(tabName); // Remove the tab name from the addedHeaders set
            }
        }
    }

    // Rebuild the itemRowMap
    itemRowMap.clear();
    for (int i = 0; i < receiptTable->rowCount(); ++i) {
        itemRowMap[receiptTable->item(i, 0)->text()] = i;
    }

    updateTotal();
}

void DropoffWindow::updateTotal()
{
    double total = 0.0;

    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        QTableWidgetItem *itemCell = receiptTable->item(row, 0);

        // Skip header rows (non-editable rows)
        if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
            continue;
        }

        // Get the price and quantity for the row
        double price = receiptTable->item(row, 1)->text().toDouble();
        QSpinBox *spin = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2));
        if (spin) {
            total += price * spin->value();
        }
    }

    // Update the total label
    totalLabel->setText(QString("Total: $%1").arg(total, 0, 'f', 2));
}

void DropoffWindow::updateCustomerInfo() {
    const Customer &customer = Session::instance().getCustomer();
    QString customerName = customer.firstName + " " + customer.lastName;
    QString customerId = customer.id;

    if (!customerName.trimmed().isEmpty() && !customerId.isEmpty()) {
        customerNameEdit->setText(customerName + " (" + customerId + ")");
    } else {
        customerNameEdit->setText("No customer selected");
    }
}

void DropoffWindow::updateDateTime()
{
    QString currentDateTime = QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss AP");
    dateTimeDisplay->setText(currentDateTime);
}