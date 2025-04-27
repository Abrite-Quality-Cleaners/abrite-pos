#include "StoreSelectionWindow.h"
#include "Store.h"
#include "User.h"
#include "Session.h"

#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QDebug>

static const QString STORE_BUTTON_STYLESHEET = "QPushButton {width:300px; height:150px; font-size:24px;}";

StoreSelectionWindow::StoreSelectionWindow(QWidget *parent)
    : QMainWindow(parent),
      userButton(new QPushButton(this)) // Initialize userButton
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    setWindowTitle("Store Selection");
    resize(1280, 1024); // Set window size
    setCentralWidget(central);
    
    userButton->setFixedHeight(userButton->size().height());
    mainLayout->addWidget(userButton, 0, Qt::AlignRight);

    qint64 storeButtonHeight = (size().height() - userButton->height()) / 3;
    QString STORE_BUTTON_STYLESHEET = "QPushButton {width:300px; height:" + QString::number(storeButtonHeight) + "px; font-size:24px;}";

    // Create a grid layout for the store buttons
    QGridLayout *gridLayout = new QGridLayout();

    QPushButton *sparkleButton = new QPushButton("Sparkle Cleaners");
    sparkleButton->setStyleSheet(STORE_BUTTON_STYLESHEET);
    connect(sparkleButton, &QPushButton::clicked, this, [this]() {
        Store::instance().setSelectedStore("Sparkle");
        //Session::instance().setDatabase("mongodb://localhost:27017", "SparkleCleaners");
        Session::instance().getMongoManager().changeDatabase("SparkleCleaners");
        emit storeSelected();
    });

    QPushButton *abriteButton = new QPushButton("Abrite Deliveries");
    abriteButton->setStyleSheet(STORE_BUTTON_STYLESHEET);
    connect(abriteButton, &QPushButton::clicked, this, [this]() {
        Store::instance().setSelectedStore("Abrite Deliveries");
        //Session::instance().setDatabase("mongodb://localhost:27017", "AbriteDeliveries");
        Session::instance().getMongoManager().changeDatabase("AbriteDeliveries");
        emit storeSelected();
    });

    QPushButton *communityButton = new QPushButton("");
    communityButton->setStyleSheet(STORE_BUTTON_STYLESHEET);
    
    QPushButton *westportButton = new QPushButton("Dump Databases");
    westportButton->setStyleSheet(STORE_BUTTON_STYLESHEET);

    QPushButton *financesButton = new QPushButton("Finances");
    financesButton->setStyleSheet(STORE_BUTTON_STYLESHEET);

    QPushButton *pricesButton = new QPushButton("Prices");
    pricesButton->setStyleSheet(STORE_BUTTON_STYLESHEET);

    gridLayout->addWidget(sparkleButton,   0, 0, 1, 1);
    gridLayout->addWidget(abriteButton,    0, 1, 1, 1);
    gridLayout->addWidget(communityButton, 1, 0, 1, 1);
    gridLayout->addWidget(westportButton,  1, 1, 1, 1);
    gridLayout->addWidget(financesButton,  2, 0, 1, 1);
    gridLayout->addWidget(pricesButton,    2, 1, 1, 1);


    mainLayout->addLayout(gridLayout, 1);

    // Connect the login success signal to the updateUserMenu slot
    connect(&Session::instance(), &Session::userUpdated, this, &StoreSelectionWindow::updateUserMenu);

    // Set up the user menu
    setupUserMenu();
}

void StoreSelectionWindow::setupUserMenu() {
    const QString &username = Session::instance().getUser().getUsername();
    qDebug() << "Setting up user menu for user:" << username;

    userMenu = new QMenu(this);
    QAction *logoutAction = new QAction("Logout", this);
    userMenu->addAction(logoutAction);

    connect(logoutAction, &QAction::triggered, this, &StoreSelectionWindow::logoutRequested);
    connect(userButton, &QPushButton::clicked, this, [this]() {
        userMenu->exec(userButton->mapToGlobal(QPoint(0, userButton->height())));
    });

    updateUserMenu();
}

void StoreSelectionWindow::updateUserMenu() {
    userButton->setText("⮌ Logout " + Session::instance().getUser().getUsername());
}