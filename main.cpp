// THIS is a quick example I wrote.
// I use the libusb "synchronous" interface. Read about that here:
//      https://libusb.sourceforge.io/api-1.0/libusb_io.html
//
// Examples from libusb authors:
// List devices:
//      https://github.com/libusb/libusb/blob/master/examples/listdevs.c
// Cypress EZUSB example:
//      main: https://github.com/libusb/libusb/blob/master/examples/fxload.c
//      lib .c: https://github.com/libusb/libusb/blob/master/examples/ezusb.c
//      lib .h: https://github.com/libusb/libusb/blob/master/examples/ezusb.h
#include <cstdio>
#include <libusb.h>
#include <stdint.h>
#include <assert.h>

typedef uint8_t u8;

static const char* lookup_usb_class(uint8_t bDeviceClass)
{ // return human-readable string describing the USB class
    switch(bDeviceClass)
    {
        case(LIBUSB_CLASS_AUDIO):        return("audio");
        case(LIBUSB_CLASS_COMM):         return("communication");
        case(LIBUSB_CLASS_HID):          return("human-interface");
        case(LIBUSB_CLASS_PHYSICAL):     return("physical");
        case(LIBUSB_CLASS_IMAGE):        return("image");
        case(LIBUSB_CLASS_PRINTER):      return("printer");
        case(LIBUSB_CLASS_MASS_STORAGE): return("mass-storage");
        case(LIBUSB_CLASS_HUB):          return("hub");
        case(LIBUSB_CLASS_DATA):         return("data");
        case(LIBUSB_CLASS_SMART_CARD):   return("smart-card");
        case(LIBUSB_CLASS_VIDEO):        return("video");
        default: return("  ?");
    }
}

static void print_all_descriptors(libusb_device_descriptor *desc)
{ // print every value in the descriptor struct
    if (desc->bDescriptorType == LIBUSB_DT_DEVICE)
    { // this be a description of a USB device, see what it says
        printf("\tUSB{");
        printf(" 'spec':%04x,", desc->bcdUSB); // USB specification
        printf(" 'class':%u", desc->bDeviceClass); // USB-IF class

        // print USB-IF class as its human readable name
        printf("(%s),",lookup_usb_class(desc->bDeviceClass));

        printf(" 'subclass':%u,", desc->bDeviceSubClass); // USB-IF subclass
        printf("\n\t    "); // new line for clean output
        printf(" 'protocol':%u,", desc->bDeviceProtocol); // USB-IF protocol
        printf(" 'max-packet-size':%u,", desc->bMaxPacketSize0); // endpoint0 max packet size
        printf("\n\t    "); // new line for clean output
        printf(" 'vendor-id':%04x,", desc->idVendor); // USB-IF vendor ID
        printf(" 'product-id':%04x,", desc->idProduct); // USB-IF product ID
        printf(" 'release-number(bcd)':%04x,", desc->bcdDevice); // in binary-coded decimal
        printf("\n\t    "); // new line for clean output
        printf(" 'manufacturer-index':%u,", desc->iManufacturer); // index to string
        printf(" 'serial-number-index':%u,", desc->iSerialNumber); // index to string
        printf("\n\t    "); // new line for clean output
        printf(" 'num-possible-configs':%u,", desc->bNumConfigurations); // number of possible configurations
        printf(" }\n"); // new line for next device
    }
}

static const char* lookup_usb_speed(libusb_device* dev)
{ // return USB speed as a human-readable string
    int speed = libusb_get_device_speed(dev);
    const bool verbose = false;
    if (verbose)
    {
        switch(speed)
        {
            case(LIBUSB_SPEED_UNKNOWN): return("unknown");
            case(LIBUSB_SPEED_LOW):     return("low(    1.5Mbps)");
            case(LIBUSB_SPEED_FULL):    return("full(    12Mbps)");
            case(LIBUSB_SPEED_HIGH):    return("high(   480Mbps)");
            case(LIBUSB_SPEED_SUPER):   return("super( 5000Mbps)");
            case(LIBUSB_SPEED_SUPER_PLUS): return("super plus(10000Mbps)");
            default: return "?";
        }
    }
    else
    {
        switch(speed)
        {
            case(LIBUSB_SPEED_UNKNOWN):     return("  unknown");
            case(LIBUSB_SPEED_LOW):         return("  1.5Mbps");
            case(LIBUSB_SPEED_FULL):        return("   12Mbps");
            case(LIBUSB_SPEED_HIGH):        return("  480Mbps");
            case(LIBUSB_SPEED_SUPER):       return(" 5000Mbps");
            case(LIBUSB_SPEED_SUPER_PLUS):  return("10000Mbps");
            default: return "?";
        }
    }
}

static void print_devs(libusb_device **devs)
{ // Print list of USB devices to stdout.
    // See https://github.com/libusb/libusb/blob/master/examples/listdevs.c

    libusb_device *dev;
    // Keep going until you run out of devices
    u8 i = 0;
    while ( (dev = devs[i++]) )
    {
        // Stop looping if there are more than 255 devices.
        if (i > sizeof(u8)*255) return;

        struct libusb_device_descriptor desc;
        int err;

        err = libusb_get_device_descriptor(dev, &desc);
        if (err != LIBUSB_SUCCESS)
        {
            printf("libusb: ERROR %d - Failed to get device descriptor.\n", err);
            return;
        }

        /* print_all_descriptors(&desc); */
        printf("\nUSB class: %s | ", lookup_usb_class(desc.bDeviceClass));
        printf("speed: %s | ", lookup_usb_speed(dev));
        uint8_t endpoint=0; // TODO: figure out which endpoint to look at
        int packet_size;
        packet_size = libusb_get_max_iso_packet_size(dev, endpoint);
        if ((packet_size != LIBUSB_ERROR_NOT_FOUND) || (packet_size != LIBUSB_ERROR_OTHER))
        {
            printf("max iso packet size: %u | ", libusb_get_max_iso_packet_size(dev, endpoint));
        }
        else
        {
            printf("cannot read packet size | ");
        }
        uint8_t busnum = libusb_get_bus_number(dev);
        uint8_t devaddr = libusb_get_device_address(dev);
        printf("%04x:%04x (bus %d, device %d)", desc.idVendor, desc.idProduct, busnum, devaddr);

        // 

        // For example, this is the output for the Chromation devkit:
        // 0403:6015 (bus 1, device 7)

        uint8_t port_numbers[8]; // depth limit is 7 as of USB 3.0
        int port_numbers_len = sizeof(port_numbers);
        int num_elements_filled = libusb_get_port_numbers(dev, port_numbers, port_numbers_len);
        if (num_elements_filled > 0)
        {
            printf(" | ");
            // Print the first part of the port number: e..g, 12
            printf("port number: %2d", port_numbers[0]);
            // Print any trailing part, e.g., .4
            for (int elem=1; elem < num_elements_filled; elem++)
            {
                printf(".%d", port_numbers[elem]);
            }
        }
    }
}

int main(int argc, char **argv)
{
    /* =====[ SETUP ]===== */
    // Make a context to avoid interfering with other libusb applications
    // https://libusb.sourceforge.io/api-1.0/libusb_contexts.html
    libusb_context *ctx;
    // Initialize:
    //      libusb_init(&ctx)
    // BEFORE calling any libusb functions
    // and DO NOT call any libusb functions AFTER deinitialization.

    int err;
    err = libusb_init(&ctx);
    if (err != LIBUSB_SUCCESS)
    {
        printf("libusb: Failed to initialize.\n");
        return -1;
    }

    /* =====[ DO A THING ]===== */
    ssize_t ndev;           // number of devices
    libusb_device **devs;   // list of devices
    ndev = libusb_get_device_list(ctx, &devs);
    printf("Thar be %llu USB devices.\n", ndev);
    print_devs(devs);
    /* =====[ CLEANUP ]===== */
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return 0;
}
// vim:fdm=syntax:
