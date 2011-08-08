/* arch/arm/mach-msm/board-legend-wifi.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Dmitry Shmidt <dimitrysh@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/err.h>
#include <asm/mach-types.h>
#include <linux/wl12xx.h>

extern int legend_wifi_set_carddetect(int val);
extern int legend_wifi_power(int on);
extern int legend_wifi_reset(int on);

#define NR_MSM_IRQS 64 /* irqs.h */
#define MSM_GPIO_TO_INT(n) (NR_MSM_IRQS + (n))
#define LEGEND_WIFI_IRQ_GPIO	        (29) /*can be found in wifi_off_gpio_table[], in board-legend-mmc.c */

struct wl12xx_platform_data wl12xx_data = {
	.set_power		= legend_wifi_power,
	.set_reset		= legend_wifi_reset,
	.set_carddetect	= legend_wifi_set_carddetect,
	.board_ref_clock	= WL12XX_REFCLOCK_26,/*From tiwlan.ini STRFRefClock = 1  # Unit: Options 5'bXX000 : Bit 0,1,2 - (0: 19.2MHz; 1: 26MHz; 2: 38.4MHz  (Default); 3: 52MHz;  4: 38.4MHz XTAL) ;*/
	.use_eeprom = false,
	.irq = MSM_GPIO_TO_INT(LEGEND_WIFI_IRQ_GPIO),
};

static struct platform_device wifi_ctrl_dev = {
	.name		= "msm_wifi",
	.id		= 1,
	.num_resources	= 0,
	.resource	= NULL,
	.dev		= {
		.platform_data = &wl12xx_data,
	},
};


static int __init legend_wifi_init(void)
{
	int ret;

	if (!machine_is_legend())
		return 0;

	printk("%s: start\n", __func__);
	ret = platform_device_register(&wifi_ctrl_dev);

	return ret;
}



late_initcall(legend_wifi_init);



