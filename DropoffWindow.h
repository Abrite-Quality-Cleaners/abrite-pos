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

class DropoffWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DropoffWindow(QWidget *parent = nullptr);
    ~DropoffWindow();

private:
    void initPrinter();
    void closePrinter();
    void loadTicketId(const QString &storeIniPath);
    void updateTicketIdDisplay();
    void handleSubmit();
    void loadPricesFromIni(const QString &filename);
    QWidget *createCategoryTab(const QList<QPair<QString, double>> &items);
    void addItemToReceipt(const QString &name, double price);
    void removeItem(int row);
    void updateTotal();

    QLineEdit *ticketIdDisplay;
    QLineEdit *customerNameEdit;
    QLineEdit *dateTimeDisplay;
    QTableWidget *receiptTable;
    QTabWidget *tabWidget;
    QLabel *totalLabel;
    QMap<QString, int> itemRowMap;

    int ticketId;
    QString ticketFile = "ticket_id.txt";
};

#endif // DROPOFFWINDOW_H