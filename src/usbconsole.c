#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <console/console.h>



void usbconsole(void) {

#if defined(CONFIG_USB_DEVICE_STACK)
        const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        uint32_t dtr = 0;

        if (usb_enable(NULL)) {
                return;
        }

        while (!dtr) {
                uart_line_ctrl_get(uart_dev, UART_LINE_CTRL_DTR, &dtr);
                k_msleep(100);
        }
        printk("uart console is open\n");
#else
        printk("uart console is disabled\n");
#endif


}
