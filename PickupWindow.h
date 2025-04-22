#ifndef PICKUPWINDOW_H
#define PICKUPWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

class PickupWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit PickupWindow(QWidget *parent = nullptr);

signals:
    void pickupDone(); // Signal emitted when the Cancel button is clicked

private slots:
    void handleCheckout();
    void updateCustomerInfo();

private:
    QLineEdit *ticketIdDisplay;
    QLineEdit *customerNameEdit;
    QTableWidget *receiptTable;
    QLabel *totalLabel;
    QTextEdit *notesEdit;
};

#endif // PICKUPWINDOW_H