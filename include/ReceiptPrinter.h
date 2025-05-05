#ifndef RECEIPT_PRINTER_H
#define RECEIPT_PRINTER_H

#include <libusb-1.0/libusb.h>
#include <QString>

class ReceiptPrinter {
public:
    ReceiptPrinter();
    ~ReceiptPrinter();

    // Initialize the printer connection
    bool init();
    
    // Close the printer connection
    void close();
    
    // Print text to the receipt
    bool printText(const QString& text);
    
    // Open the cash drawer
    bool openDrawer();
    
    // Cut the paper
    bool cutPaper();

private:      
    libusb_context* ctx;
    libusb_device_handle* handle;
    
    // Helper methods
    bool sendPrintText(const uint8_t* msg, int len);
    bool sendOpenDrawerCommand();
    bool sendCutCommand();
};

#endif // RECEIPT_PRINTER_H