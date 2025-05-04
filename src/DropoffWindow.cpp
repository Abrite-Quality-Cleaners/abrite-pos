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
#include <QDateTimeEdit>
#include <QDebug>
#include <QStyle>
#include <QTimer>
#include "Session.h"

DropoffWindow::DropoffWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // Tab widget on the left
    tabWidget = new QTabWidget(this);

    mainLayout->addWidget(tabWidget, 2); // Stretch factor of 2 for 2/3 width

    // Right-side layout for customer info, date/time, receipt, and buttons
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Customer Info Section
    QHBoxLayout *customerRow = new QHBoxLayout();
    QLabel *customerLabel = new QLabel("Customer:", this);
    customerLabel->setStyleSheet("font-weight: bold;"); // Bold the label
    customerNameEdit = new QLineEdit(this);
    customerNameEdit->setFixedWidth(400);
    customerNameEdit->setReadOnly(true);
    customerNameEdit->setPlaceholderText("No customer selected");

    customerRow->addWidget(customerLabel);
    customerRow->addWidget(customerNameEdit);
    customerRow->addStretch();
    rightLayout->addLayout(customerRow);

    // Dropoff Date/Time Section
    QHBoxLayout *dateTimeRow = new QHBoxLayout();
    QLabel *dropoffDateLabel = new QLabel("Dropoff Date/Time:", this);
    dropoffDateLabel->setStyleSheet("font-weight: bold;"); // Bold the label
    dateTimeDisplay = new QLineEdit(this);
    dateTimeDisplay->setReadOnly(true);
    dateTimeDisplay->setFixedWidth(300);

    dateTimeRow->addWidget(dropoffDateLabel);
    dateTimeRow->addWidget(dateTimeDisplay);
    dateTimeRow->addStretch();
    rightLayout->addLayout(dateTimeRow);

    // Pickup Date Section
    QHBoxLayout *pickupDateRow = new QHBoxLayout();
    QLabel *pickupDateLabel = new QLabel("Pickup Date:", this);
    pickupDateLabel->setStyleSheet("font-weight: bold;"); // Bold the label
    QDateEdit *pickupDatePicker = new QDateEdit(QDate::currentDate().addDays(3), this);
    pickupDatePicker->setDisplayFormat("yyyy-MM-dd");
    pickupDatePicker->setCalendarPopup(true);

    pickupDateRow->addWidget(pickupDateLabel);
    pickupDateRow->addWidget(pickupDatePicker);
    pickupDateRow->addStretch();
    rightLayout->addLayout(pickupDateRow);

    // Receipt Table
    receiptTable = new QTableWidget(this);
    receiptTable->setColumnCount(4);
    receiptTable->setHorizontalHeaderLabels({"Item", "Price", "Quantity", ""});
    receiptTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receiptTable->setSelectionMode(QAbstractItemView::NoSelection);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    receiptTable->verticalHeader()->setVisible(false);
    receiptTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rightLayout->addWidget(receiptTable);

    // Total Label
    totalLabel = new QLabel("Total: $0.00", this);
    totalLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    rightLayout->addWidget(totalLabel);

    // Payment Method Section
    QHBoxLayout *paymentMethodRow = new QHBoxLayout();
    QLabel *paymentMethodLabel = new QLabel("Payment Method:", this);
    paymentMethodLabel->setStyleSheet("font-weight: bold;"); // Bold the label
    paymentMethodEdit = new QLineEdit("On-pickup", this);
    paymentMethodEdit->setReadOnly(true);
    paymentMethodEdit->setFixedWidth(200);

    paymentMethodRow->addWidget(paymentMethodLabel);
    paymentMethodRow->addWidget(paymentMethodEdit);
    paymentMethodRow->addStretch();
    rightLayout->addLayout(paymentMethodRow);

    // Notes Textbox
    notesEdit = new QTextEdit(this);
    notesEdit->setPlaceholderText("Enter notes about the order...");
    notesEdit->setFixedHeight(100);
    rightLayout->addWidget(notesEdit);

    // Buttons Row
    QHBoxLayout *btnRow = new QHBoxLayout();

    // Check-out Button
    QPushButton *checkoutButton = new QPushButton("Check-out", this);
    checkoutButton->setMinimumWidth(100);
    btnRow->addWidget(checkoutButton);
    connect(checkoutButton, &QPushButton::clicked, this, [=]() {
        handleCheckout();
    });

    // Pay Button
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

            // Update the payment method text box
            paymentMethodEdit->setText(paymentMethod);
        }
    });

    // Cancel Button
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setMinimumWidth(100);
    btnRow->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, [=]() {
        emit dropoffDone();
    });

    rightLayout->addLayout(btnRow);

    // Add the right layout to the main layout
    mainLayout->addLayout(rightLayout, 1); // Stretch factor of 1 for 1/3 width

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

void DropoffWindow::handleCheckout()
{
    // Update the current order with the latest data
    const Customer &customer = Session::instance().getCustomer();
    currentOrder.customerId = customer.id;
    currentOrder.dropoffDate = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    currentOrder.orderNote = notesEdit->toPlainText();
    currentOrder.orderTotal = 0.0;
    currentOrder.subOrders.clear();
    
    // Build an order
    SubOrder subOrder = {0, "", {}, 0.0};
    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        QTableWidgetItem *itemCell = receiptTable->item(row, 0);

        if (!itemCell || !itemCell->flags().testFlag(Qt::ItemIsEditable)) {
            if (!subOrder.type.isEmpty()) {
                currentOrder.subOrders.append(subOrder);
                currentOrder.orderTotal += subOrder.total;
            }
            
            subOrder = {Session::instance().getMongoManager().getThenIncrementNextId(), 
                    itemCell->text(), {}, 0.0};
            continue;
        }
            
        QString itemName = itemCell->text();
        double price = receiptTable->item(row, 1)->text().toDouble();
        int qty = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2))->value();
        
        // Add the item to the order
        subOrder.total += price * qty;
        subOrder.items.append({itemName, price, qty});      
    }
    currentOrder.orderTotal += subOrder.total;
    currentOrder.subOrders.append(subOrder);

    QString orderId = Session::instance().getMongoManager().addOrder(currentOrder);
    if (!orderId.isEmpty()) {
        qDebug() << "Order added successfully with ID:" << orderId;
    } else {
        qDebug() << "Failed to add order.";
    }

    printReceipts();
}

void DropoffWindow::printReceipts() {
    const Customer &customer = Session::instance().getCustomer();
    QString client = customer.firstName + " " + customer.lastName;
    if (client.trimmed().isEmpty()) client = "Unknown";

    QString customerReceipt = QString(
        "             Sparkle Cleaners\n"
        "            165 Oak Grove Ave.\n"
        "           Fall River, MA 02720\n\n"
        "CLIENT: %1\n"
        "DROP  : %2\n"
        "PICKUP: %3\n"
        "PAY   : On-pickup\n"
    ).arg(customer.id,
            currentOrder.dropoffDate,
            currentOrder.pickupDate.isEmpty() ? "Unknown" : currentOrder.pickupDate);

    QStringList subOrderReceipts;

    double total = 0.0;

    for (const SubOrder &subOrder : currentOrder.subOrders) {
        QString subOrderReceipt = QString(
            "CLIENT: %1\n"
        ).arg(customer.id);

        QString subOrderHeader = QString(
            "------------------------------------------\n"
            "%1 [%2]\n"
            "------------------------------------------\n"
            "|GARMENT              |QUANTITY  |PRICE  |\n"
            "------------------------------------------\n"
        ).arg(subOrder.type).arg(subOrder.id);

        customerReceipt += subOrderHeader;
        subOrderReceipt += subOrderHeader;

        for (const Item &item : subOrder.items) {
            QString itemLine = QString(" %1 %2 %3\n")
                .arg(item.name.leftJustified(21))
                .arg(QString::number(item.quantity).leftJustified(10))
                .arg(QString::number(item.price, 'f', 2).leftJustified(7));

            customerReceipt += itemLine;
            subOrderReceipt += itemLine;
        }

        QString subtotal = QString(
            "                       -------------------\n"
            "                       SUBTOTAL:     $%1\n\n"
        ).arg(subOrder.total, 0, 'f', 2);

        customerReceipt += subtotal;

        subOrderReceipt += subtotal;
        subOrderReceipt += QString("NOTE: %2\n").arg(currentOrder.orderNote);

        subOrderReceipts.append(subOrderReceipt);
    }

    // Add total and notes to the consolidated receipt
    customerReceipt += QString(
        "                       -------------------\n"
        "                       TOTAL:     $%1\n\n"
        "NOTE: %2\n"
    ).arg(currentOrder.orderTotal, 0, 'f', 2).arg(currentOrder.orderNote);

    for (const QString &subOrderReceipt : subOrderReceipts) {
        printf("### Sub-Order ############################\n%s", 
                subOrderReceipt.toStdString().c_str());
    }
    printf("\n### Customer Receipt #####################\n%s",
            customerReceipt.toStdString().c_str());
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
        // Pass the type name (tab name) to createCategoryTab
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

        // Connect the button's clicked signal to addItemToReceipt with the type name
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