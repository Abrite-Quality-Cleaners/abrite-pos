#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class PaymentDialog : public QDialog {
    Q_OBJECT

public:
    explicit PaymentDialog(QWidget *parent = nullptr, double orderTotal = 0.0);

    QString getSelectedPaymentMethod() const;
    QString getCheckNumber() const;
    double getPaymentAmount() const;

    // Add setters to allow editing existing payment
    void setPaymentMethod(const QString &method);
    void setCheckNumber(const QString &number);
    void setPaymentAmount(double amount);

private:
    QPushButton *cashButton;
    QPushButton *creditButton;
    QPushButton *checkButton;
    QPushButton *storeCreditButton;
    QLineEdit *checkNumberEdit;
    QLineEdit *paymentAmountEdit;
    QLabel *orderTotalLabel;
    QComboBox *paymentMethodCombo;

    QString selectedPaymentMethod;
    QString checkNumber;
    double orderTotal;

private slots:
    void handleCash();
    void handleCredit();
    void handleCheck();
    void handleStoreCredit();
};

#endif // PAYMENTDIALOG_H