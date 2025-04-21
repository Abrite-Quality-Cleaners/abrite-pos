#include "DropoffWindow.h"
#include "Customer.h"
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

DropoffWindow::DropoffWindow(QWidget *parent)
    : QMainWindow(parent)
{
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
    dateTimeDisplay = new QLineEdit(this);
    dateTimeDisplay->setReadOnly(true);
    dateTimeDisplay->setFixedWidth(200);

    topRow->addWidget(new QLabel("Ticket ID:"));
    topRow->addWidget(ticketIdDisplay);
    topRow->addSpacing(15);
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
    receiptTable->horizontalHeader()->setStretchLastSection(false);
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
        // Add logic for check-out functionality here
    });

    // Create "Pay" button
    QPushButton *payButton = new QPushButton("Pay", this);
    payButton->setMinimumWidth(100);
    btnRow->addWidget(payButton);
    connect(payButton, &QPushButton::clicked, this, [=]() {
        qDebug() << "Pay button clicked";
        // Add logic for payment functionality here
    });

    // Create "Void" button
    QPushButton *voidButton = new QPushButton("Void", this);
    voidButton->setMinimumWidth(100);
    btnRow->addWidget(voidButton);
    connect(voidButton, &QPushButton::clicked, this, [=]() {
        qDebug() << "Void button clicked";
        receiptTable->setRowCount(0); // Clear the receipt table
        itemRowMap.clear();           // Clear the item map
        notesEdit->clear();           // Clear the notes textbox
        updateTotal();                // Update the total to $0.00
    });

    rightLayout->addLayout(btnRow);

    mainLayout->addLayout(rightLayout, 1);
    masterLayout->addLayout(mainLayout);

    setCentralWidget(central);
    resize(900, 900);
    setWindowTitle("Dropoff Interface");

    loadTicketId("store.ini");
    updateTicketIdDisplay();
    loadPricesFromIni("../prices.ini");

    // Connect the Customer singleton's signal to update the customer info
    connect(&Customer::instance(), &Customer::customerUpdated, this, &DropoffWindow::updateCustomerInfo);

    // Initialize the customer info
    updateCustomerInfo();

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

void DropoffWindow::handleSubmit()
{
    QFile f(ticketFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << ticketId + 1;
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
        "TICKET ID: %4 (Dryclean)\n"
        "PAYMENT  : On-pickup\n"
        "EMPLOYEE : NA\n\n"
        "------------------------------------------\n"
        "|GARMENT              |QUANTITY  |PRICE  |\n"
        "------------------------------------------\n"
    ).arg(client,
          QDateTime::currentDateTime().toString("ddd MM/dd/yyyy hh:mm AP"),
          QDateTime::currentDateTime().addDays(3).toString("ddd MM/dd/yyyy hh:mm AP"),
          QString::number(ticketId));

    double total = 0.0;
    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        QString item = receiptTable->item(row, 0)->text();
        double price = receiptTable->item(row, 1)->text().toDouble();
        int qty = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2))->value();
        double subtotal = qty * price;
        total += subtotal;
        receipt += QString("%1%2%3\n")
            .arg(item.leftJustified(24))
            .arg(QString::number(qty).rightJustified(10))
            .arg(QString::number(subtotal, 'f', 2).rightJustified(10));
    }

    receipt += QString("                       -------------------\n"
                       "                       TOTAL:     $%1\n\n"
                       "NOTE: <Notes>\n").arg(total, 0, 'f', 2);

    receipt += "\n\n\n\n\n\n\n"; // Add some blank lines so the cut doesn't chop off part of the receipt

    ticketId++;
    updateTicketIdDisplay();
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
        tabWidget->addTab(createCategoryTab(items), cat);
        s.endGroup();
    }
}

QWidget *DropoffWindow::createCategoryTab(const QList<QPair<QString, double>> &items)
{
    QWidget *tab = new QWidget(this);
    QGridLayout *grid = new QGridLayout(tab);
    for (int i = 0; i < items.size(); ++i) {
        QPushButton *btn = new QPushButton(QString("%1\n$%2").arg(items[i].first).arg(items[i].second, 0, 'f', 2));
        connect(btn, &QPushButton::clicked, this, [=]() {
            addItemToReceipt(items[i].first, items[i].second);
        });
        grid->addWidget(btn, i / 3, i % 3);
    }
    return tab;
}

void DropoffWindow::addItemToReceipt(const QString &name, double price)
{
    if (itemRowMap.contains(name)) {
        int row = itemRowMap[name];
        auto *spin = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2));
        spin->setValue(spin->value() + 1);
    } else {
        int row = receiptTable->rowCount();
        receiptTable->insertRow(row);
        itemRowMap[name] = row;

        receiptTable->setItem(row, 0, new QTableWidgetItem(name));
        receiptTable->setItem(row, 1, new QTableWidgetItem(QString::number(price, 'f', 2)));

        auto *spin = new QSpinBox(this);
        spin->setRange(1, 999);
        spin->setValue(1);
        receiptTable->setCellWidget(row, 2, spin);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {
            updateTotal();
        });

        auto *rmBtn = new QPushButton();
        rmBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        rmBtn->setFixedWidth(50);
        receiptTable->setCellWidget(row, 3, rmBtn);
        connect(rmBtn, &QPushButton::clicked, this, [=]() {
            for (int r = 0; r < receiptTable->rowCount(); ++r) {
                if (receiptTable->cellWidget(r, 3) == sender()) {
                    removeItem(r);
                    break;
                }
            }
        });
    }

    updateTotal();
}

void DropoffWindow::removeItem(int row)
{
    QString name = receiptTable->item(row, 0)->text();
    itemRowMap.remove(name);
    receiptTable->removeRow(row);
    itemRowMap.clear();
    for (int i = 0; i < receiptTable->rowCount(); ++i)
        itemRowMap[receiptTable->item(i, 0)->text()] = i;
    updateTotal();
}

void DropoffWindow::updateTotal()
{
    double total = 0.0;
    for (int row = 0; row < receiptTable->rowCount(); ++row) {
        double price = receiptTable->item(row, 1)->text().toDouble();
        QSpinBox *spin = qobject_cast<QSpinBox *>(receiptTable->cellWidget(row, 2));
        if (spin) {
            total += price * spin->value();
        }
    }
    totalLabel->setText(QString("Total: $%1").arg(total, 0, 'f', 2));
}

void DropoffWindow::updateCustomerInfo() {
    QString customerName = Customer::instance().getName();
    QString customerPhone = Customer::instance().getPhone();

    if (!customerName.isEmpty()) {
        customerNameEdit->setText(customerName + " (" + customerPhone + ")");
    } else {
        customerNameEdit->setText("No customer selected");
    }
}

void DropoffWindow::updateDateTime()
{
    QString currentDateTime = QDateTime::currentDateTime().toString("MM/dd/yyyy hh:mm:ss AP");
    dateTimeDisplay->setText(currentDateTime);
}