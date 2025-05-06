#ifndef PICKUPWINDOW_H
#define PICKUPWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>

class PickupWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit PickupWindow(QWidget *parent = nullptr);

signals:
    void pickupDone(); // Signal emitted when the Cancel button is clicked

public slots:
    void updateCustomerInfo();

private slots:
    void handleCheckout();
    void handlePayment();

private:
    void onOrderSelected();
    void populateOrdersTable();

    QLabel *orderIdLabel;
    QTableWidget *receiptTable;
    QLineEdit *ticketIdDisplay;
    QLineEdit *customerNameEdit;
    QTableWidget *customerOrdersTable;
    QLabel *totalLabel;
    QLineEdit *paymentMethodEdit;
    QLineEdit *amountPaidEdit;
    QTextEdit *customerNotesEdit;
    QTextEdit *notesEdit;
};

#endif // PICKUPWINDOW_H