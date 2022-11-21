#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

static bool configured;
static const struct device *hdev;
static ATOMIC_DEFINE(hid_ep_in_busy, 1);

#define HID_EP_BUSY_FLAG 0
#define REPORT_ID_1	0x01
#define REPORT_PERIOD K_SECONDS(1)

static struct report {
    uint8_t id;
    char value[8];
} __packed report_1 = {
        .id = REPORT_ID_1,
        .value = "",
};
char *report_msg;

static void send_report();

/*
 * Simple HID Report Descriptor
 * Report ID is present for completeness, although it can be omitted.
 * Output of "usbhid-dump -d 2fe3:0006 -e descriptor":
 *  05 01 09 00 A1 01 15 00    26 FF 00 85 01 75 08 95
 *  01 09 00 81 02 C0
 */
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
    int wrote;
    if (!atomic_test_and_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {
        strncpy(report_1.value, report_msg, 8);
        hid_int_ep_write(hdev, (uint8_t *)&report_1, sizeof(report_1), &wrote);
    }
}

static void int_in_ready_cb(const struct device *dev) {
    ARG_UNUSED(dev);
    atomic_test_and_clear_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG);
}

/*
 * On Idle callback is available here as an example even if actual use is
 * very limited. In contrast to report_event_handler(),
 * report value is not incremented here.
 */
static void on_idle_cb(const struct device *dev, uint16_t report_id) {
    send_report();
}

static void report_event_handler(struct k_timer *dummy) {
    send_report();
}

static const struct hid_ops ops = {
        .int_in_ready = int_in_ready_cb,
        .on_idle = on_idle_cb,
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

int ret = -1;
static void init(void) {
    ret = usb_enable(status_cb);
    if (ret != 0) {
        return;
    }
}

static void write_message(char *msg) {
    if (ret == -1) {
        init();
    }
    report_msg = msg;
    send_report();
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
