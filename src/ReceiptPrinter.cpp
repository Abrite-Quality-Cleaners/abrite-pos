#include "../include/ReceiptPrinter.h"

#include <QDebug>

static const uint16_t  EPSON_VENDOR_ID = 0x04b8;
static const uint16_t EPSON_PRODUCT_ID = 0x0202;

// ESC/POS commands
static const uint8_t  drawerCmd[5] = {0x1B, 0x70, 0x00, 0x19, 0xFA};
static const uint8_t     cutCmd[3] = {0x1D, 0x56, 0x00};
static const uint8_t  reverseOn[3] = {0x1D, 0x42, 0x01};
static const uint8_t reverseOff[3] = {0x1D, 0x42, 0x00};

ReceiptPrinter::ReceiptPrinter() : ctx(nullptr), handle(nullptr) {
}

ReceiptPrinter::~ReceiptPrinter() {
    close();
}

bool ReceiptPrinter::init() {
    int res = libusb_init(&ctx);
    if (res < 0) return false;

    handle = libusb_open_device_with_vid_pid(ctx, EPSON_VENDOR_ID, EPSON_PRODUCT_ID);
    if (!handle) {
        libusb_exit(ctx);
        ctx = nullptr;
        return false;
    }

    if (libusb_kernel_driver_active(handle, 0) == 1) {
        libusb_detach_kernel_driver(handle, 0);
    }

    res = libusb_claim_interface(handle, 0);
    if (res < 0) {
        libusb_close(handle);
        libusb_exit(ctx);
        handle = nullptr;
        ctx = nullptr;
        return false;
    }

    return true;
}

void ReceiptPrinter::close() {
    if (handle) {
        libusb_release_interface(handle, 0);
        libusb_close(handle);
        handle = nullptr;
    }
    if (ctx) {
        libusb_exit(ctx);
        ctx = nullptr;
    }
}

bool ReceiptPrinter::printText(const QString& text) {
    if (!handle) return false;
    // Add new lines so text isn't cutoff
    QString paddedText = text + "\n\n\n\n\n\n";
    return sendPrintText(reinterpret_cast<const uint8_t*>(paddedText.toUtf8().data()), paddedText.toUtf8().size());
}

bool ReceiptPrinter::openDrawer() {
    if (!handle) return false;
    return sendOpenDrawerCommand();
}

bool ReceiptPrinter::cutPaper() {
    if (!handle) return false;
    return sendCutCommand();
}

bool ReceiptPrinter::sendPrintText(const uint8_t* msg, int len) {
    if (!handle) return false;
    int transferred = 0;
    return libusb_bulk_transfer(handle, 0x01, const_cast<uint8_t*>(msg), len, &transferred, 0) == 0;
}

bool ReceiptPrinter::sendOpenDrawerCommand() {
    if (!handle) return false;
    int transferred = 0;
    return libusb_bulk_transfer(handle, 0x01, const_cast<uint8_t*>(drawerCmd), sizeof(drawerCmd), &transferred, 0) == 0;
}

bool ReceiptPrinter::sendCutCommand() {
    if (!handle) return false;
    int transferred = 0;
    return libusb_bulk_transfer(handle, 0x01, const_cast<uint8_t*>(cutCmd), sizeof(cutCmd), &transferred, 0) == 0;
} 