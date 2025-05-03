#ifndef DROPOFFWINDOW_H
#define DROPOFFWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QMap>
#include <QTextEdit>
#include <QTimer>
#include <QSet>
#include "Order.h"

class DropoffWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DropoffWindow(QWidget *parent = nullptr);
    ~DropoffWindow();

signals:
    void dropoffDone(); // Signal emitted when the Cancel button is clicked

public slots:
    void updateCustomerInfo(); // Slot to update customer info

private slots:
    void updateDateTime(); // Slot to update the date and time

private:
    void initPrinter();
    void closePrinter();
    void loadTicketId(const QString &storeIniPath);
    void handleCheckout(); // Refactored method
    void loadPricesFromIni(const QString &filename);
    QWidget *createCategoryTab(const QString &categoryName, const QList<QPair<QString, double>> &items);
    void addItemToReceipt(const QString &tabName, const QString &itemName, double price);
    void removeItem(int row);
    void updateTotal();
    QString createReceipt(); // New method to generate the receipt

    QLineEdit *customerNameEdit;
    QLineEdit *dateTimeDisplay;
    QTableWidget *receiptTable;
    QTabWidget *tabWidget;
    QLabel *totalLabel;
    QLabel *paymentMethodLabel; // Label for payment method
    QMap<QString, int> itemRowMap;
    QTextEdit *notesEdit; // Textbox for order notes
    QTimer *dateTimeTimer; // Timer to update the date and time
    QSet<QString> addedHeaders; // Tracks which tab headers have been added

    int ticketId;
    QString ticketFile = "ticket_id.txt";

    Order currentOrder; // Order object to keep track of the current order
};

#endif // DROPOFFWINDOW_H