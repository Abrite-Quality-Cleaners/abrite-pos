#ifndef STORESELECTIONWINDOW_H
#define STORESELECTIONWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMenu>
#include <QPushButton>

class StoreSelectionWindow : public QMainWindow
{
    Q_OBJECT

public:
    StoreSelectionWindow(QWidget *parent = nullptr);

signals:
    void storeSelected();
    void logoutRequested();

private slots:
    void updateUserMenu(); // Slot to update the user menu

private:
    void setupUserMenu();

    QPushButton *userButton;
    QMenu       *userMenu;
};

#endif // STORESELECTIONWINDOW_H
