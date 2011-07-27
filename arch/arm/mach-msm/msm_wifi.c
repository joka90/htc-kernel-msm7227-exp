/*
 * platform driver for msm_wifi device
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * Copyright (C) 2008 Google Inc
 */
#include <linux/platform_device.h>
#include <linux/spi/wl12xx.h>
#include <linux/proc_fs.h>

extern unsigned char *get_wifi_nvs_ram(void);
static struct proc_dir_entry *msm_wifi_calibration;

#define WIFI_NVS_LEN_OFFSET     0x0C
#define WIFI_NVS_DATA_OFFSET    0x40
#define WIFI_NVS_MAX_SIZE       0x800UL

static unsigned long tiwlan_get_nvs_size( void )
{
    unsigned char *ptr;
    unsigned long len;

    ptr = get_wifi_nvs_ram();
    if( ptr == NULL ) {
        return 0;
    }
    /* Size in format LE assumed */
    memcpy( (void *)&len, (void *)(ptr + WIFI_NVS_LEN_OFFSET), sizeof(len) );
    len = min( len, (WIFI_NVS_MAX_SIZE-WIFI_NVS_DATA_OFFSET) );
    return len;
}

static int tiwlan_calibration_read_proc(char *page, char **start, off_t off,
                                int count, int *eof, void *data)
{
    unsigned char *ptr;
    unsigned long len;

    ptr = get_wifi_nvs_ram();
    if( ptr == NULL ) {
        return 0;
    }
    len = tiwlan_get_nvs_size();
    /* i += sprintf(page+i, "WiFi Calibration Size = %lu %x bytes\n", len); */
    memcpy( (void *)page, (void *)(ptr + WIFI_NVS_DATA_OFFSET), len );
    return len;
}

static int tiwlan_calibration_write_proc(struct file *file, const char *buffer,
                                 unsigned long count, void *data)
{
    return 0;
}

static int wifi_probe(struct platform_device *pdev)
{
	struct wl12xx_platform_data *wifi_ctrl = pdev->dev.platform_data;

	printk(KERN_DEBUG "wifi probe start\n");

	if (!wifi_ctrl)
		return -ENODEV;

	if (wifi_ctrl->set_power)
		wifi_ctrl->set_power(1);	/* Power On */
	if (wifi_ctrl->set_reset)
		wifi_ctrl->set_reset(0);	/* Reset clear */
	if (wifi_ctrl->set_carddetect)
		wifi_ctrl->set_carddetect(1);	/* CardDetect (0->1) */

	printk(KERN_DEBUG "wifi probe done\n");
	return 0;
}

static int wifi_remove(struct platform_device *pdev)
{
	struct wl12xx_platform_data *wifi_ctrl = pdev->dev.platform_data;

	printk(KERN_DEBUG "wifi remove start\n");
	if (!wifi_ctrl)
		return -ENODEV;

	if (wifi_ctrl->set_carddetect)
		wifi_ctrl->set_carddetect(0);	/* CardDetect (1->0) */
	if (wifi_ctrl->set_reset)
		wifi_ctrl->set_reset(1);	/* Reset active */
	if (wifi_ctrl->set_power)
		wifi_ctrl->set_power(0);	/* Power Off */

	printk(KERN_DEBUG "wifi remove end\n");
	return 0;
}

static struct platform_driver wifi_device = {
	.probe		= wifi_probe,
	.remove		= wifi_remove,
	.driver		= {
		.name   = "msm_wifi",
	},
};

static int __init msm_wifi_sdio_init(void)
{
        msm_wifi_calibration = create_proc_entry("msm_wifi_calibration", 0644, NULL);
	if (msm_wifi_calibration) {
		msm_wifi_calibration->size = tiwlan_get_nvs_size();
		msm_wifi_calibration->read_proc = tiwlan_calibration_read_proc;
		msm_wifi_calibration->write_proc = tiwlan_calibration_write_proc;
	}
	return platform_driver_register(&wifi_device);
}

static void __exit msm_wifi_sdio_exit(void)
{
	platform_driver_unregister(&wifi_device);
	if(msm_wifi_calibration)      /* After renaming */
		remove_proc_entry("msm_wifi_calibration", NULL);
}

module_init(msm_wifi_sdio_init);
module_exit(msm_wifi_sdio_exit);
MODULE_LICENSE("GPL");
