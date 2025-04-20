#include "WindowController.h"

#include "LoginWindow.h"
#include "StoreSelectionWindow.h"
#include "ClientSelectionWindow.h"
#include "DropoffWindow.h"

WindowController::WindowController(QObject *parent)
    : QObject(parent),
      loginWindow(new LoginWindow),
      storeWindow(new StoreSelectionWindow),
      clientSelWindow(new ClientSelectionWindow),
      dropoffWindow(new DropoffWindow) // Initialize DropoffWindow
{
    // Connect signals to slots
    connect(loginWindow, &LoginWindow::loginSuccess, this, &WindowController::onLoginSuccess);
    connect(storeWindow, &StoreSelectionWindow::storeSelected, this, &WindowController::onStoreSelected);
    connect(storeWindow, &StoreSelectionWindow::logoutRequested, this, &WindowController::onLogoutRequested);
    connect(clientSelWindow, &ClientSelectionWindow::dropOffRequested, this, &WindowController::onDropOffRequested); // Connect Drop-off signal
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
    clientSelWindow->show();
}

void WindowController::onLogoutRequested()
{
    storeWindow->hide();
    loginWindow->show();
}

void WindowController::onDropOffRequested()
{
    clientSelWindow->hide(); // Hide the ClientSelectionWindow
    dropoffWindow->show();   // Show the DropoffWindow
}