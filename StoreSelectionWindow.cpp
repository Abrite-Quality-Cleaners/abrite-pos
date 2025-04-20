#include "StoreSelectionWindow.h"
#include "User.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QDebug>

static const QStringList stores = {"Sparkle", "Abrite Deliveries"};

StoreSelectionWindow::StoreSelectionWindow(QWidget *parent, qint64 width, qint64 height)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QGridLayout *layout = new QGridLayout(central);

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

    // Set up the user menu
    setupUserMenu();

    // Connect the User::instance() signal to the updateUserMenu slot
    connect(&User::instance(), &User::userUpdated, this, &StoreSelectionWindow::updateUserMenu);
}

void StoreSelectionWindow::setupUserMenu()
{
    // Create the user button
    userButton = new QPushButton(this);
    userButton->setStyleSheet("font-size: 16px; padding: 5px;");
    userButton->setCursor(Qt::PointingHandCursor);

    qDebug() << "Setting up user menu for user:" << User::instance().getUsername();

    // Create the dropdown menu
    userMenu = new QMenu(this);
    QAction *logoutAction = new QAction("Logout", this);
    userMenu->addAction(logoutAction);

    // Connect the logout action
    connect(logoutAction, &QAction::triggered, this, [this]() {
        emit logoutRequested();
    });

    // Show the menu when the button is clicked
    connect(userButton, &QPushButton::clicked, this, [this]() {
        userMenu->exec(userButton->mapToGlobal(QPoint(0, userButton->height())));
    });

    // Add the button to the top-right corner
    QWidget *topRightWidget = new QWidget(this);
    QHBoxLayout *topRightLayout = new QHBoxLayout(topRightWidget);
    topRightLayout->addWidget(userButton);
    topRightLayout->setAlignment(Qt::AlignRight);
    topRightWidget->setLayout(topRightLayout);

    QGridLayout *mainLayout = qobject_cast<QGridLayout *>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(topRightWidget, 0, stores.length() - 1, Qt::AlignRight);
    }

    // Initialize the user menu
    updateUserMenu();
}

void StoreSelectionWindow::updateUserMenu()
{
    QString username = User::instance().getUsername();
    qDebug() << "Updating user menu with username:" << username;

    QStringList nameParts = username.split(' ');
    QString displayName = nameParts.first();
    if (nameParts.size() > 1) {
        displayName += " " + QString(nameParts.last().at(0)) + ".";
    }
    userButton->setText(displayName + " ▼");
}
