#include "WindowController.h"

#include "LoginWindow.h"
#include "StoreSelectionWindow.h"

WindowController::WindowController(QObject *parent)
    : QObject(parent),
      loginWindow(new LoginWindow),
      storeWindow(new StoreSelectionWindow)
{
    // Connect signals to slots
    connect(loginWindow, &LoginWindow::loginSuccess, this, &WindowController::onLoginSuccess);
    connect(storeWindow, &StoreSelectionWindow::storeSelected, this, &WindowController::onStoreSelected);
    connect(storeWindow, &StoreSelectionWindow::logoutRequested, this, &WindowController::onLogoutRequested);
}
 
void WindowController::start()
{
    loginWindow->show();
}

void WindowController::onLoginSuccess()
{
    loginWindow->hide();
    storeWindow->show();
}

void WindowController::onStoreSelected()
{
    storeWindow->hide();
}

void WindowController::onLogoutRequested()
{
    storeWindow->hide();
    loginWindow->show();
}