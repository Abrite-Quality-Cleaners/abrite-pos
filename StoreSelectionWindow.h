#ifndef STORESELECTIONWINDOW_H
#define STORESELECTIONWINDOW_H

#include <QMainWindow>
#include <QWidget>

class StoreSelectionWindow : public QMainWindow
{
    Q_OBJECT

public:
    StoreSelectionWindow(QWidget *parent = nullptr, qint64 width=100, qint64 height=100);

signals:
    void storeSelected();
};

#endif // STORESELECTIONWINDOW_H
