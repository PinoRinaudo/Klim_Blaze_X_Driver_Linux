#include <iostream>
#include <libusb.h>

#define VENDOR_ID 0x260d
#define PRODUCT_ID 0x1074
#define INTERFACE_NUM 1
#define ALT_INTERFACE_NUM 0
class KlimBlazeX
{
private:
    static KlimBlazeX *instance;
    libusb_context *context;
    libusb_device_handle *handle;
    uint16_t vendorId;
    uint16_t productId;
    int16_t lastError;

    KlimBlazeX(uint16_t vendorId, uint16_t productId)
        : context(nullptr), handle(nullptr)
    {
        this->vendorId = vendorId;
        this->productId = productId;
        bind();
        set20MinInactivity();
    }

    uint8_t claimInterface()
    {
        int lastError = LIBUSB_SUCCESS;
        if (libusb_kernel_driver_active(handle, INTERFACE_NUM))
        {
            lastError = libusb_detach_kernel_driver(handle, INTERFACE_NUM);
            if (lastError != LIBUSB_SUCCESS)
                return lastError;
        }

        lastError = libusb_claim_interface(handle, INTERFACE_NUM);
        return lastError == LIBUSB_SUCCESS;
    }

    uint8_t releaseInterface()
    {
        int lastError = libusb_release_interface(handle, INTERFACE_NUM);
        if (lastError != LIBUSB_SUCCESS)
            return false;
        lastError = libusb_attach_kernel_driver(handle, INTERFACE_NUM);
        if (lastError != LIBUSB_SUCCESS)
            return false;
        return true;
    }

    uint8_t sendData(uint8_t *data, size_t len)
    {
        if (claimInterface())
        {
            if (libusb_control_transfer(handle, 0x21, 0x09, 0x0308, 0x0001, data, len, 200) != len)
            {
                releaseInterface();
                return false;
            }
        }
        return releaseInterface();
    }

public:
    static KlimBlazeX *getInstance(uint16_t vendorId = VENDOR_ID, uint16_t productId = PRODUCT_ID)
    {
        if (!instance)
            instance = new KlimBlazeX(vendorId, productId);
        return instance;
    }

    ~KlimBlazeX()
    {
        releaseInterface();
        libusb_close(handle);
        if (context)
            libusb_exit(context);
    }

    uint8_t setNeon(uint8_t async = false)
    {
        uint8_t data[] = {0x08, 0x07, 0x00, 0x00, 0xa0, 0x07, 0x03, 0x00, 0xff, 0xf8, 0x96, 0x96, 0x2f, 0x00, 0x00, 0x00, 0x4a};
        return sendData(data, sizeof(data));
    }

    uint8_t setStream()
    {
        uint8_t data[] = {0x08, 0x07, 0x00, 0x00, 0xa0, 0x07, 0x00, 0x00, 0xff, 0xf8, 0x03, 0xfc, 0x5f, 0x00, 0x00, 0x00, 0x4a};
        return sendData(data, sizeof(data));
    }

    uint8_t setColor(uint8_t r, uint8_t g, uint8_t b)
    {
        uint8_t parity_bit = 85 - (r + g + b) % 256;
        uint8_t light = 0xff;
        uint8_t speed = 0xff; // maybe speed
        uint8_t data[] = {0x08, 0x07, 0x00, 0x00, 0xa0, 0x07, 0x02, r, g, b, light, speed, parity_bit, 0x00, 0x00, 0x00, 0x4a};
        return sendData(data, sizeof(data));
    }

    uint8_t set20MinInactivity()
    {
        uint8_t data[] = {0x08, 0x07, 0x00, 0x00, 0xad, 0x02, 0x78, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42};
        return sendData(data, sizeof(data));
    }

    uint8_t bind()
    {
        if (handle)
            libusb_close(handle);
        if (context)
            libusb_exit(context);
        lastError = libusb_init(&context);
        if (lastError != LIBUSB_SUCCESS)
            return false;

        handle = libusb_open_device_with_vid_pid(context, vendorId, productId);
        if (!handle)
        {
            lastError = -99;
            return false;
        }

        lastError = libusb_set_interface_alt_setting(handle, INTERFACE_NUM, ALT_INTERFACE_NUM);
        return true;
    }

    uint8_t isBind()
    {
        if (!handle)
            return false;
        return libusb_kernel_driver_active(handle, INTERFACE_NUM) == 1;
    }

    int16_t getLastErrorCode()
    {
        return lastError;
    }
    std::string getMessageError()
    {
        const char *error_message = libusb_strerror(lastError);
        if (error_message)
            return std::string(error_message);
        else
            return "Unknown error";
    }
};
KlimBlazeX *KlimBlazeX::instance = nullptr;
