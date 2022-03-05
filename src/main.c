#include <errno.h>
#include <zephyr.h>
#include <sys/byteorder.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/nus.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>

#include <settings/settings.h>


#include <logging/log.h>

#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <console/console.h>


#include "src/appdef.h"

#define LOG_MODULE_NAME blebeacon
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


#define NAME_LEN 81
#define ADR_LEN  18


static bool data_cb(struct bt_data *data, void *user_data)
{
        char *name = user_data;
        uint8_t len;

        switch (data->type) {
        case BT_DATA_NAME_COMPLETE:
        case BT_DATA_NAME_SHORTENED:
                len = MIN(data->data_len, NAME_LEN - 1);
                memcpy(name, data->data, len);
                name[len] = '\0';
                return false;
        default:
                return true;
        }
}


static void scan_recv(const struct bt_le_scan_recv_info *info,
                      struct net_buf_simple *buf)
{
        char addr[ADR_LEN];

        char name[NAME_LEN];

        (void)memset(name, 0, sizeof(name));
        bt_data_parse(buf, data_cb, name);

        bt_addr_le_to_str(info->addr, addr, sizeof(addr));
	int rssi = info->rssi;
        if ( strlen(name) > 0 ) {
          printk("received bcast from: %s  rssi: %d  name: %s\n", addr, rssi,name);
	}


}



static struct bt_le_scan_cb scan_callbacks = {
        .recv = scan_recv,
};


static int scan_init(void)
{
	int err = 0;

	struct bt_le_scan_param scan_param = {
            .type = BT_LE_SCAN_TYPE_PASSIVE,
            .options = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
            .interval = 0x0010,
            .window = 0x0010,
        };

	struct bt_scan_init_param scan_init = {
                .scan_param = &scan_param,
		.connect_if_match = 1,
	};


	bt_scan_init(&scan_init);
        bt_le_scan_cb_register(&scan_callbacks);

	return err;
}



void main(void)
{
	int err;


#if defined(CONFIG_USB_DEVICE_STACK)
	usbconsole();
#endif
	printk("Starting BLE beacon scan\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Error: Bluetooth init (code %d)", err);
		return;
	}

        bt_le_scan_cb_register(&scan_callbacks);


	printk("Bluetooth initialized\n");

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

        err = scan_init();
	if (err) {
	      printk("Error: scan  init (code  %d)\n", err);
	      return;
	}

	printk("Starting Bluetooth scan\n");


	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		printk("Error: scanning failed (code %d)", err);
		return;
	}


}
