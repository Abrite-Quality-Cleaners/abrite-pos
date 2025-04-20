#ifndef CLIENTSELECTIONWINDOW_H
#define CLIENTSELECTIONWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

class ClientSelectionWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientSelectionWindow(QWidget *parent = nullptr);

signals:
    void dropOffRequested(); // Signal emitted when the Drop-off button is clicked

private slots:
    void onSearch(); // Slot to handle search functionality
    void onRowSelected(); // Slot to enable buttons when a row is selected
    void onDropOffClicked(); // Slot to handle Drop-off button click

private:
    void searchCsv(const QString &filePath, const QString &firstName, const QString &lastName, const QString &phone, const QString &ticket);

    QLineEdit *firstNameEdit; // First Name input
    QLineEdit *lastNameEdit;  // Last Name input
    QLineEdit *phoneEdit;
    QLineEdit *ticketEdit;
    QPushButton *searchButton;
    QTableWidget *resultTable;
    QPushButton *dropOffButton; // Drop-off button
    QPushButton *pickUpButton;  // Pick-up button
};

#endif // CLIENTSELECTIONWINDOW_H