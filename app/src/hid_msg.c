#include "main.h"

#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#include <string.h>

static bool configured;
static const struct device *hdev;
static ATOMIC_DEFINE(hid_ep_in_busy, 1);

#define HID_EP_BUSY_FLAG 0
#define REPORT_ID_1	0x01

static struct report {
    uint8_t id;
    char value[8];
} __packed report_1 = {
        .id = REPORT_ID_1,
        .value = "",
};

static void hid_init();
static void write_message(char *msg);

static const uint8_t hid_report_desc[] = {
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
        HID_USAGE(HID_USAGE_GEN_DESKTOP_UNDEFINED),
        HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_LOGICAL_MIN8(0x00),
        HID_LOGICAL_MAX16(0xFF, 0x00),
        HID_REPORT_ID(REPORT_ID_1),
        HID_REPORT_SIZE(64),
        HID_REPORT_COUNT(1),
        HID_USAGE(HID_USAGE_GEN_DESKTOP_UNDEFINED),
        HID_INPUT(0x02),
        HID_END_COLLECTION,
};

static void send_report() {
    if (!atomic_test_and_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {
        int ret, wrote;
        ret = hid_int_ep_write(hdev, (uint8_t *)&report_1, sizeof(report_1), &wrote);
        if (ret != 0) {
            /*
             * Do nothing and wait until host has reset the device
             * and hid_ep_in_busy is cleared.
             */
            LOG_ERR("Failed to submit report");
        } else {
            LOG_DBG("Report submitted");
        }
    }
}

static void int_in_ready_cb(const struct device *dev) {
    ARG_UNUSED(dev);
    if (!atomic_test_and_clear_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {
        LOG_WRN("IN endpoint callback without preceding buffer write");
    }
}

static const struct hid_ops ops = {
        .int_in_ready = int_in_ready_cb,
};

static void status_cb(enum usb_dc_status_code status, const uint8_t *param) {
    switch (status) {
        case USB_DC_RESET:
            configured = false;
            break;
        case USB_DC_CONFIGURED:
            if (!configured) {
                int_in_ready_cb(hdev);
                configured = true;
            }
            break;
        case USB_DC_SOF:
            break;
        default:
            break;
    }
}

int is_usb_enabled = -1;
static void hid_init(void) {
    is_usb_enabled = usb_enable(status_cb);
    if (is_usb_enabled != 0) {
        LOG_ERR("Failed to enable USB");
        return;
    }
}

static void write_message(char msg[]) {
    if (is_usb_enabled != 0) {
        LOG_ERR("USB was not enabled");
        return;
    }
    for (int i = 0; i < strlen(msg); i += 7) {
        strncpy(report_1.value, msg + i, 7);
        send_report();
        k_msleep(50);
    }
}

static int composite_pre_init(const struct device *dev) {
    hdev = device_get_binding("HID_0");
    if (hdev == NULL) {
        return -ENODEV;
    }
    usb_hid_register_device(hdev, hid_report_desc, sizeof(hid_report_desc), &ops);

    atomic_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG);

    usb_hid_set_proto_code(hdev, HID_BOOT_IFACE_CODE_NONE);

    return usb_hid_init(hdev);
}

SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);