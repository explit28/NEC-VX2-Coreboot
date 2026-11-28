/* SPDX-License-Identifier: GPL-2.0-only */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ops.h>
#include <device/cardbus.h>
#include <types.h>

/*
 * I don't think this code is quite correct but it is close.
 * Anyone with a cardbus bridge and a little time should be able
 * to make it usable quickly. -- Eric Biederman 24 March 2005
 */

/*
 * IO should be max 256 bytes. However, since we may have a P2P bridge below
 * a cardbus bridge, we need 4K.
 */
#define CARDBUS_IO_SIZE		4096
#define CARDBUS_MEM_SIZE	(32 * 1024 * 1024)

struct cardbus_window {
	unsigned int base_reg;
	unsigned int limit_reg;
	unsigned long type;
	unsigned int address_mask;
};

static const struct cardbus_window cardbus_windows[] = {
	{ PCI_CB_MEMORY_BASE_0, PCI_CB_MEMORY_LIMIT_0,
	  IORESOURCE_MEM | IORESOURCE_PREFETCH, ~0xfffU },
	{ PCI_CB_MEMORY_BASE_1, PCI_CB_MEMORY_LIMIT_1,
	  IORESOURCE_MEM, ~0xfffU },
	{ PCI_CB_IO_BASE_0, PCI_CB_IO_LIMIT_0,
	  IORESOURCE_IO, PCI_CB_IO_RANGE_MASK },
	{ PCI_CB_IO_BASE_1, PCI_CB_IO_LIMIT_1,
	  IORESOURCE_IO, PCI_CB_IO_RANGE_MASK },
};

static void cardbus_record_bridge_resource(struct device *dev, resource_t moving,
		resource_t min_size, unsigned int index, unsigned long type)
{
	struct resource *resource;
	unsigned long gran;
	resource_t step;

	/* Initialize the constraints on the current bus. */
	resource = NULL;
	if (!moving)
		return;

	resource = new_resource(dev, index);
	resource->size = 0;
	gran = 0;
	step = 1;
	while ((moving & step) == 0) {
		gran += 1;
		step <<= 1;
	}
	resource->gran = gran;
	resource->align = gran;
	resource->limit = moving | (step - 1);
	resource->flags = type | IORESOURCE_PCI_BRIDGE | IORESOURCE_BRIDGE;

	/* Don't let the minimum size exceed what we can put in the resource. */
	if ((min_size - 1) > resource->limit)
		min_size = resource->limit + 1;

	resource->size = min_size;
}

void cardbus_read_resources(struct device *dev)
{
	resource_t moving_base, moving_limit, moving;
	unsigned long type;
	u16 ctl;

	/* See if needs a card control registers base address. */

	pci_get_resource(dev, PCI_BASE_ADDRESS_0);

	compact_resources(dev);

	/* See which bridge I/O resources are implemented. */
	moving_base = pci_moving_config32(dev, PCI_CB_IO_BASE_0);
	moving_limit = pci_moving_config32(dev, PCI_CB_IO_LIMIT_0);
	moving = moving_base & moving_limit & PCI_CB_IO_RANGE_MASK;

	/* Initialize the I/O space constraints on the current bus. */
	cardbus_record_bridge_resource(dev, moving, CARDBUS_IO_SIZE,
				       PCI_CB_IO_BASE_0, IORESOURCE_IO);

	/* See which bridge I/O resources are implemented. */
	moving_base = pci_moving_config32(dev, PCI_CB_IO_BASE_1);
	moving_limit = pci_moving_config32(dev, PCI_CB_IO_LIMIT_1);
	moving = moving_base & moving_limit & PCI_CB_IO_RANGE_MASK;

	/* Initialize the I/O space constraints on the current bus. */
	cardbus_record_bridge_resource(dev, moving, CARDBUS_IO_SIZE,
				       PCI_CB_IO_BASE_1, IORESOURCE_IO);

	/* If I can, enable prefetch for mem0. */
	ctl = pci_read_config16(dev, PCI_CB_BRIDGE_CONTROL);
	ctl &= ~PCI_CB_BRIDGE_CTL_PREFETCH_MEM0;
	ctl &= ~PCI_CB_BRIDGE_CTL_PREFETCH_MEM1;
	ctl |= PCI_CB_BRIDGE_CTL_PREFETCH_MEM0;
	pci_write_config16(dev, PCI_CB_BRIDGE_CONTROL, ctl);
	ctl = pci_read_config16(dev, PCI_CB_BRIDGE_CONTROL);

	/* See which bridge memory resources are implemented. */
	moving_base = pci_moving_config32(dev, PCI_CB_MEMORY_BASE_0);
	moving_limit = pci_moving_config32(dev, PCI_CB_MEMORY_LIMIT_0);
	moving = moving_base & moving_limit & PCI_CB_MEMORY_RANGE_MASK;

	/* Initialize the memory space constraints on the current bus. */
	type = IORESOURCE_MEM;
	if (ctl & PCI_CB_BRIDGE_CTL_PREFETCH_MEM0)
		type |= IORESOURCE_PREFETCH;
	cardbus_record_bridge_resource(dev, moving, CARDBUS_MEM_SIZE,
				       PCI_CB_MEMORY_BASE_0, type);

	/* See which bridge memory resources are implemented. */
	moving_base = pci_moving_config32(dev, PCI_CB_MEMORY_BASE_1);
	moving_limit = pci_moving_config32(dev, PCI_CB_MEMORY_LIMIT_1);
	moving = moving_base & moving_limit & PCI_CB_MEMORY_RANGE_MASK;

	/* Initialize the memory space constraints on the current bus. */
	cardbus_record_bridge_resource(dev, moving, CARDBUS_MEM_SIZE,
				       PCI_CB_MEMORY_BASE_1, IORESOURCE_MEM);

	compact_resources(dev);
}

static void cardbus_disable_window(struct device *dev,
				  const struct cardbus_window *window,
				  struct resource *resource)
{
	u32 base = resource->limit & window->address_mask;
	u32 base_type = 0;
	u32 limit_type = 0;

	if (window->type & IORESOURCE_IO) {
		base_type = pci_read_config32(dev, window->base_reg) & ~window->address_mask;
		limit_type = pci_read_config32(dev, window->limit_reg) & ~window->address_mask;
	}

	/* Make the base higher than the limit while the window is inactive. */
	pci_write_config32(dev, window->limit_reg, limit_type);
	pci_write_config32(dev, window->base_reg, base | base_type);
}

static bool cardbus_window_end(const struct resource *resource, resource_t *end)
{
	resource_t gran_mask, size;

	if (resource->gran >= 32 || resource->align >= 32)
		return false;

	gran_mask = (1ULL << resource->gran) - 1;
	if (resource->base & ((1ULL << resource->align) - 1))
		return false;

	if (resource->size > (~0ULL - gran_mask))
		return false;

	size = (resource->size + gran_mask) & ~gran_mask;
	if (!size || size > (1ULL << 32) || resource->base > (1ULL << 32) - size)
		return false;

	*end = resource->base + size - 1;
	return *end <= resource->limit;
}

static void cardbus_program_window(struct device *dev,
				   const struct cardbus_window *window,
				   struct resource *resource)
{
	resource_t end;
	u32 base_type = 0;
	u32 limit_type = 0;
	u32 base, limit;
	bool enabled;

	if (window->type & IORESOURCE_IO) {
		base_type = pci_read_config32(dev, window->base_reg) & ~window->address_mask;
		limit_type = pci_read_config32(dev, window->limit_reg) & ~window->address_mask;
	}

	enabled = (resource->flags & IORESOURCE_ASSIGNED) && resource->size &&
		cardbus_window_end(resource, &end);
	if (!enabled && (resource->flags & IORESOURCE_ASSIGNED) && resource->size)
		printk(BIOS_ERR, "%s: invalid CardBus window at 0x%lx, disabling it\n",
		       dev_path(dev), resource->index);

	if (enabled) {
		base = resource->base & window->address_mask;
		limit = end & window->address_mask;
		dev->command |= (window->type & IORESOURCE_IO) ?
			PCI_COMMAND_IO : PCI_COMMAND_MEMORY;
	} else {
		base = resource->limit & window->address_mask;
		limit = 0;
	}

	/* Keep the window closed until its inclusive limit is programmed. */
	pci_write_config32(dev, window->limit_reg, limit | limit_type);
	pci_write_config32(dev, window->base_reg, base | base_type);
	resource->flags |= IORESOURCE_STORED;
	if (enabled)
		report_resource_stored(dev, resource, "");
}

void cardbus_set_resources(struct device *dev)
{
	unsigned long flags[ARRAY_SIZE(cardbus_windows)];
	struct resource *resources[ARRAY_SIZE(cardbus_windows)];
	u16 command;

	/* Keep old windows from decoding while the new ranges are installed. */
	command = pci_read_config16(dev, PCI_COMMAND);
	command &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
	pci_write_config16(dev, PCI_COMMAND, command);
	dev->command &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY);

	/* First close every implemented window before changing any range. */
	for (size_t i = 0; i < ARRAY_SIZE(cardbus_windows); i++) {
		resources[i] = probe_resource(dev, cardbus_windows[i].base_reg);
		if (!resources[i])
			continue;
		flags[i] = resources[i]->flags;
		cardbus_disable_window(dev, &cardbus_windows[i], resources[i]);

		/* Let the common PCI path program BARs and the command register. */
		resources[i]->flags &= ~IORESOURCE_PCI_BRIDGE;
		resources[i]->flags |= IORESOURCE_FIXED;
	}

	pci_dev_set_resources(dev);

	for (size_t i = 0; i < ARRAY_SIZE(cardbus_windows); i++) {
		if (!resources[i])
			continue;
		resources[i]->flags = flags[i];
		cardbus_program_window(dev, &cardbus_windows[i], resources[i]);
	}
}

void cardbus_enable_resources(struct device *dev)
{
	u16 ctrl;

	ctrl = pci_read_config16(dev, PCI_CB_BRIDGE_CONTROL);
	ctrl |= (dev->downstream->bridge_ctrl & (
			PCI_BRIDGE_CTL_VGA |
			PCI_BRIDGE_CTL_MASTER_ABORT |
			PCI_BRIDGE_CTL_BUS_RESET));
	/* Error check */
	ctrl |= (PCI_CB_BRIDGE_CTL_PARITY | PCI_CB_BRIDGE_CTL_SERR);
	printk(BIOS_DEBUG, "%s bridge ctrl <- %04x\n", dev_path(dev), ctrl);
	pci_write_config16(dev, PCI_CB_BRIDGE_CONTROL, ctrl);

	pci_dev_enable_resources(dev);
}

struct device_operations default_cardbus_ops_bus = {
	.read_resources   = cardbus_read_resources,
	.set_resources    = cardbus_set_resources,
	.enable_resources = cardbus_enable_resources,
	.scan_bus         = pci_scan_bridge,
	.reset_bus        = pci_bus_reset,
};
