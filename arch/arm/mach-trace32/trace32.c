/*
 *  arch/arm/mach-vt8500/vt8500.c
 *
 *  Copyright (C) 2012 Tony Prisk <linux@prisktech.co.nz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/io.h>
#include <linux/pm.h>
#include <linux/reboot.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>

static struct map_desc trace32_io_desc[] __initdata = {
	/* SoC MMIO registers */
	[0] = {
		.virtual	= 0xfe000000,
		.pfn		= __phys_to_pfn(0xff000000),
		.length		= 0xffffff, /* max of all chip variants */
		.type		= MT_DEVICE
	},
};

static void __init trace32_map_io(void)
{
	iotable_init(trace32_io_desc, ARRAY_SIZE(trace32_io_desc));
}

static const char * const trace32_dt_compat[] = {
	"TRACE32,simulator",
	NULL
};

DT_MACHINE_START(TRACE32, "TRACE32 simulator (Device Tree)")
	.dt_compat	= trace32_dt_compat,
	.map_io		= trace32_map_io,
MACHINE_END

