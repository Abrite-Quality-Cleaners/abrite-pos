#ifndef CLIENTSELECTIONWINDOW_H
#define CLIENTSELECTIONWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include "Customer.h"

class ClientSelectionWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientSelectionWindow(QWidget *parent = nullptr);

signals:
    void dropOffRequested(); // Signal emitted when Drop-off is clicked
    void pickUpRequested();  // Signal emitted when Pick-up is clicked

private slots:
    void onSearch();         // Slot to handle search functionality
    void onRowSelected();    // Slot to enable buttons when a row is selected
    void onDropOffClicked(); // Slot to handle Drop-off button click
    void onPickUpClicked();  // Slot to handle Pick-up button click
    void onAddCustomerClicked(); // Slot to handle Add Customer button click
    void onEditCustomerClicked(); // Slot to handle Edit Customer button click

private:
    void searchCsv(const QString &filePath, const QString &firstName, const QString &lastName, const QString &phone, const QString &ticket);

    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *phoneEdit;
    QLineEdit *ticketEdit;
    QPushButton *searchButton;
    QTableWidget *resultTable;
    QPushButton *dropOffButton; // Drop-off button
    QPushButton *pickUpButton;  // Pick-up button
    QPushButton *addCustomerButton; // Add Customer button
    QPushButton *editCustomerButton; // Edit Customer button

    QList<Customer> customers;
};

#endif // CLIENTSELECTIONWINDOW_H