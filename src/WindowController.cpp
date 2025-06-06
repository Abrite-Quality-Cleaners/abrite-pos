#include "WindowController.h"

#include "LoginWindow.h"
#include "StoreSelectionWindow.h"
#include "ClientSelectionWindow.h"
#include "DropoffWindow.h"
#include "PickupWindow.h"

WindowController::WindowController(QObject *parent)
    : QObject(parent),
      loginWindow(new LoginWindow),
      storeWindow(new StoreSelectionWindow),
      clientSelWindow(new ClientSelectionWindow),
      dropoffWindow(new DropoffWindow),
      pickupWindow(new PickupWindow) // Initialize PickupWindow
{
    // Connect signals to slots
    connect(loginWindow, &LoginWindow::loginSuccess, this, &WindowController::onLoginSuccess);
    connect(storeWindow, &StoreSelectionWindow::storeSelected, this, &WindowController::onStoreSelected);
    connect(storeWindow, &StoreSelectionWindow::logoutRequested, this, &WindowController::onLogoutRequested);
    connect(clientSelWindow, &ClientSelectionWindow::dropOffRequested, this, &WindowController::onDropOffRequested); // Connect Drop-off signal
    connect(clientSelWindow, &ClientSelectionWindow::pickUpRequested, this, &WindowController::onPickUpRequested); // Connect Pick-up signal
    connect(dropoffWindow, &DropoffWindow::dropoffDone, this, &WindowController::onDropoffDone); // Connect dropoffDone signal
    connect(pickupWindow, &PickupWindow::pickupDone, this, &WindowController::onPickupDone); // Connect pickupDone signal

    // Connect the dropOffRequested signal from ClientSelectionWindow to the updateCustomerInfo slot in DropoffWindow
    connect(clientSelWindow, &ClientSelectionWindow::dropOffRequested, dropoffWindow, &DropoffWindow::updateCustomerInfo);
    connect(clientSelWindow, &ClientSelectionWindow::pickUpRequested, pickupWindow, &PickupWindow::updateCustomerInfo);
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

void WindowController::onDropoffDone()
{
    dropoffWindow->hide(); // Hide the DropoffWindow
    storeWindow->show();   // Show the StoreSelectionWindow
}

void WindowController::onPickUpRequested()
{
    clientSelWindow->hide(); // Hide the ClientSelectionWindow
    pickupWindow->show();    // Show the PickupWindow
}

void WindowController::onPickupDone()
{
    pickupWindow->hide(); // Hide the PickupWindow
    clientSelWindow->show(); // Show the ClientSelectionWindow
}