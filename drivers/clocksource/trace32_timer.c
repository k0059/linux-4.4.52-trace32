/*
 * Rockchip timer support
 *
 * Copyright (C) Daniel Lezcano <daniel.lezcano@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clk.h>
#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/cpumask.h>
#include <linux/sched_clock.h>

#define TIMER_NAME "trace32_timer"

static irqreturn_t simulator_timer_interrupt(int irq, void *dev_id);
static int simulator_timer_set_next_event(unsigned int cycles, struct clock_event_device *evt);
static int simulator_timer_set_periodic(struct clock_event_device *evt);
static int simulator_timer_shutdown(struct clock_event_device *evt);

static void *base;
static struct clock_event_device simulator_clockevent = {
	.set_next_event = simulator_timer_set_next_event,
	.shift = 0x20,
	.state_use_accessors = CLOCK_EVT_STATE_DETACHED,
	.features = 3,
	.set_state_periodic = simulator_timer_set_periodic,
	.set_state_shutdown = simulator_timer_shutdown,
	.tick_resume = simulator_timer_shutdown,
	.name = "TIMER",
	.rating = 0x12c,
};
static struct irqaction simulator_timer_irq = {
	.handler = simulator_timer_interrupt,
	.flags = 0x15200,
	.name = "Simulator Timer Tick",
};

static int simulator_timer_shutdown(struct clock_event_device *evt)
{
  writel(2, (void*)0xFE001008);
  writel(1000, (void*)0xFE001000);
  return 0;
}

static int simulator_timer_set_periodic(struct clock_event_device *evt)
{
  writel(1000, (void*)0xFE001000);
  writel(3, (void*)0xFE001008);
  return 0;
}

static int simulator_timer_set_next_event(unsigned int cycles, struct clock_event_device *evt)
{
	writel(cycles, (void*)0xfe001000);
	return 0;
}

extern void tick_handle_periodic(struct clock_event_device *dev);
static irqreturn_t simulator_timer_interrupt(int irq, void *dev_id)
{
	writel(1, (void*)0xfe00100c);
	tick_handle_periodic(&simulator_clockevent);
	return IRQ_HANDLED;
}

u64 simulator_read_sched_clock(void)
{
	u64 ret;
	
	ret = readl((void*)0xfe001050);
	dsb();
	return ret;
}

static void __init trace32_timer_init(struct device_node *np)
{
	int ret, irq;

	base = of_iomap(np, 0);
	if (!base) {
		pr_err("Failed to get base address for '%s'\n", TIMER_NAME);
		return;
	}

	irq = irq_of_parse_and_map(np, 0);
	if (irq <= 0) {
		pr_err("Failed to get irq for '%s'\n", TIMER_NAME);
		return;
	}
	
	writel(0, (void*)(0xfe001000));
	writel(0, (void*)(0xfe001008));
	writel(1, (void*)(0xfe00100c));
	writel(1000, (void*)(0xfe001000));
	writel(2, (void*)(0xfe001008));

	setup_irq(irq, &simulator_timer_irq);
	if ( clocksource_mmio_init(
         (void *)0xFE001050,
         "simulator_clkevt",
         0xF4240u,
         300,
         0x18u,
         clocksource_mmio_readl_up))
  {
    pr_err("Failed to mmio init for '%s'\n", TIMER_NAME);
    return;
  }
	
	sched_clock_register(simulator_read_sched_clock, 24, 0xf4240);
	simulator_clockevent.cpumask = (const struct cpumask*)cpu_bit_bitmap[1];
	clockevents_config_and_register(&simulator_clockevent, 0xf4240, 1, UINT_MAX);
}
CLOCKSOURCE_OF_DECLARE(trace32_timer, "TRACE32,simulator-timer", trace32_timer_init);
