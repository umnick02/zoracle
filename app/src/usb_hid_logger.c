#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_core.h>
#include <zephyr/logging/log_output.h>
#include <zephyr/logging/log_backend_std.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

static struct k_spinlock probe_lock;

static uint32_t probe_log_format_current = LOG_OUTPUT_TEXT;

#define LOG_BUF_SIZE 127
static uint8_t log_buf[LOG_BUF_SIZE];

static bool configured;
static const struct device *hdev;
static ATOMIC_DEFINE(hid_ep_in_busy, 1);
int is_usb_enabled = -1;

#define HID_EP_BUSY_FLAG 0

static struct report {
    char value[8];
} __packed log_report = {
        .value = "",
};

static const uint8_t hid_report_desc[] = {
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
        HID_USAGE(HID_USAGE_GEN_DESKTOP_UNDEFINED),
        HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_LOGICAL_MIN8(0x00),
        HID_LOGICAL_MAX16(0xFF, 0x00),
        HID_REPORT_SIZE(64),
        HID_REPORT_COUNT(1),
        HID_USAGE(HID_USAGE_GEN_DESKTOP_UNDEFINED),
        HID_INPUT(0x02),
        HID_END_COLLECTION,
};

static void send_report() {
    int ret, wrote;
    if (!atomic_test_and_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {
        ret = hid_int_ep_write(hdev, (uint8_t * ) & log_report, sizeof(log_report), &wrote);
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

static int write_message(uint8_t *data, size_t length, void *ctx) {
    if (is_usb_enabled != 0) {
        LOG_ERR("USB was not enabled");
        return 0;
    }
    for (int i = 0; i < length; i += 7) {
        strncpy(log_report.value, data + i, 7);
        log_report.value[7] = '\0';
        send_report();
        k_msleep(10);
    }
    return length;
}

LOG_OUTPUT_DEFINE(log_output_usb_hid_probe, write_message, log_buf, sizeof(log_buf));

static void probe_log_dropped(const struct log_backend *const backend, uint32_t cnt) {
    log_output_dropped_process(&log_output_usb_hid_probe, cnt);
}

static uint32_t format_flags(void) {
    uint32_t flags = LOG_OUTPUT_FLAG_TIMESTAMP | LOG_OUTPUT_FLAG_LEVEL;
    return flags;
}

static void probe_log_process(const struct log_backend *const backend, union log_msg_generic *msg) {
    log_format_func_t log_output_func = log_format_func_t_get(probe_log_format_current);
    k_spinlock_key_t key = k_spin_lock(&probe_lock);
    log_output_func(&log_output_usb_hid_probe, &msg->log, format_flags());
    k_spin_unlock(&probe_lock, key);
}

static void probe_log_panic(struct log_backend const *const backend) {
    k_spinlock_key_t key = k_spin_lock(&probe_lock);
    log_backend_std_panic(&log_output_usb_hid_probe);
    k_spin_unlock(&probe_lock, key);
}

static int probe_log_format_set(const struct log_backend *const backend, uint32_t log_type) {
    probe_log_format_current = log_type;
    return 0;
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

static void hid_init() {
    is_usb_enabled = usb_enable(status_cb);
    if (is_usb_enabled != 0) {
        LOG_ERR("Failed to enable USB");
        return;
    }
}

static void probe_log_init(const struct log_backend *const backend) {
    ARG_UNUSED(backend);
}

SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
const struct log_backend_api log_backend_usb_hid_api = {
        .process = probe_log_process,
        .dropped = IS_ENABLED(CONFIG_LOG_MODE_IMMEDIATE) ? NULL : probe_log_dropped,
        .panic = probe_log_panic,
        .format_set = probe_log_format_set,
        .init = probe_log_init,
};
LOG_BACKEND_DEFINE(log_backend_usb_hid, log_backend_usb_hid_api, false);
