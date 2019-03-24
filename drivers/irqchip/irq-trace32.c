/*
 * Allwinner A1X SoCs IRQ chip driver.
 *
 * Copyright (C) 2012 Maxime Ripard
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Benn Huang <benn@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/irqdomain.h>
#include <linux/irqdesc.h>

#include <asm/exception.h>
#include <asm/mach/irq.h>

static void simulator_irqchip_handle_irq(struct pt_regs *regs);
static int simulator_irqdomain_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hwirq);
static void simulator_irqdomain_unmap(struct irq_domain *d, unsigned int irq);
static void simulator_irq_mask(struct irq_data *d);
static void simulator_irq_unmask(struct irq_data *d);
static int simulator_irq_set_type(struct irq_data *d, unsigned int trigger);

struct simulator_irq_data{
	void *base;
	struct irq_domain *domain;
};

static struct irq_domain_ops simulator_irqdomain_ops = {
	.map = simulator_irqdomain_map,
	.unmap = simulator_irqdomain_unmap,
	.xlate = irq_domain_xlate_onetwocell,
};
static struct simulator_irq_data girq;
static struct irq_chip simulator_irq_chip = {
	.name = "TRACE32-SIMULATOR",
	.irq_ack = simulator_irq_mask,
	.irq_mask = simulator_irq_mask,
	.irq_unmask = simulator_irq_unmask,
	.irq_set_type = simulator_irq_set_type,
};

static int simulator_irq_set_type(struct irq_data *d, unsigned int trigger)
{
  struct irq_common_data *v2; // r3
  int result; // r0

  if ( trigger & 4 )
    v2 = d->common;
  result = 0;
  if ( trigger & 4 )
    v2[3].state_use_accessors = (unsigned int)handle_level_irq;
  return result;
}

static void simulator_irq_unmask(struct irq_data *d)
{
  struct simulator_irq_data *v1; // r3

  v1 = d->chip_data;
  dsb();
  writel(1, v1->base + 12);
}

static void simulator_irq_mask(struct irq_data *d)
{
  struct simulator_irq_data *v1; // r3

  v1 = d->chip_data;
  dsb();
  writel(1, v1->base + 16);
}

static void simulator_irqdomain_unmap(struct irq_domain *d, unsigned int irq)
{
  unsigned int v2; // r4

  v2 = irq;
  irq_set_chip_and_handler(irq, 0, 0);
  irq_set_chip_data(v2, 0);
}

static int simulator_irqdomain_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hwirq)
{
  unsigned int v3; // r4

  v3 = irq;
  irq_set_chip_data(irq, d->host_data);
  irq_set_chip_and_handler(v3, &simulator_irq_chip, (irq_flow_handler_t)handle_level_irq);
  //irq_modify_status(v3, 0x400u, 0);
  irq_clear_status_flags(irq, IRQ_NOPROBE);
  return 0;
}

static void simulator_irqchip_handle_irq(struct pt_regs *regs)
{
  struct pt_regs *i; // r5
  unsigned int v2; // r1
  int v3; // r3

  for ( i = regs; ; handle_domain_irq(girq.domain, v2, i) )
  {
    v3 = readl(girq.base + 4);
    dsb();
    if ( !v3 )
      break;
    v2 = readl(girq.base);
    dsb();
  }
}

static int __init trace32_of_init(struct device_node *node,
				struct device_node *parent)
{
	girq.base = of_iomap(node, 0);
	if (!girq.base)
		panic("%s: unable to map IC registers\n",
			node->full_name);
	dsb(0xf);
	/* Disable all interrupts */
	writel(0, girq.base + 0x10);

	girq.domain = irq_domain_add_simple(node, 4, 0,
						 &simulator_irqdomain_ops, &girq);

	set_handle_irq(simulator_irqchip_handle_irq);

	return 0;
}
IRQCHIP_DECLARE(trace32_ic, "TRACE32,simulator-interrupt-controller", trace32_of_init);

