#include "StoreSelectionWindow.h"

#include <QGridLayout>
#include <QSpacerItem>
#include <QPushButton>

static const QStringList stores = {"Sparkle", "Abrite Deliveries"};

StoreSelectionWindow::StoreSelectionWindow(QWidget *parent, qint64 width, qint64 height)
    : QMainWindow(parent)
{
    QWidget     *central = new QWidget(this);
    QGridLayout *layout  = new QGridLayout(central);
    layout->addItem(new QSpacerItem(10, 50), 0, 0, 1, stores.length());
    layout->addItem(new QSpacerItem(10, 50), 2, 0, 1, stores.length());

    qint64 store_index = 0;
    for (const QString &s : stores) {
        QPushButton *storeButton = new QPushButton(s);
        storeButton->setFixedSize(300, 150);
        storeButton->setStyleSheet("font-size: 24px");
        layout->addWidget(storeButton, 1, store_index++, 1, 1);
    }

    setWindowTitle("Store Selection");
    resize(width, height);
    setCentralWidget(central);
}
