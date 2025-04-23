#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>

class PaymentDialog : public QDialog {
    Q_OBJECT

public:
    explicit PaymentDialog(QWidget *parent = nullptr);

    QString getSelectedPaymentMethod() const;
    QString getCheckNumber() const;

private:
    QPushButton *cashButton;
    QPushButton *creditButton;
    QPushButton *checkButton;
    QPushButton *storeCreditButton;
    QLineEdit *checkNumberEdit;

    QString selectedPaymentMethod;
    QString checkNumber;

private slots:
    void handleCash();
    void handleCredit();
    void handleCheck();
    void handleStoreCredit();
};

#endif // PAYMENTDIALOG_H