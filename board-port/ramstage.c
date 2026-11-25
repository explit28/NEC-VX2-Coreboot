/* SPDX-License-Identifier: GPL-2.0-only */
/* NEC VersaPro VX-2: pre-FSP-S GPIO hook. */
#include <bootstate.h>
#include <device/pci.h>
#include <soc/ramstage.h>
#include "gpio.h"

void mainboard_silicon_init_params(FSP_S_CONFIG *params)
{
	params->PcieRpFunctionSwap = 0;
	mainboard_configure_late_gpios();
}

/* Apply the OEM CardBus setup after PCI resource assignment. */
static void configure_cardbus(void *unused)
{
	const struct device *root_port = pcidev_path_on_root(PCI_DEVFN(0x1c, 1));
	if (!root_port || !root_port->enabled || root_port->vendor != 0x8086 ||
	    root_port->device != 0x9d11 || !root_port->downstream)
		return;

	const struct device *cardbus = pcidev_path_behind(root_port->downstream, PCI_DEVFN(0, 0));
	if (!cardbus || !cardbus->enabled || cardbus->vendor != 0x1217 ||
	    cardbus->device != 0x8134)
		return;

	pci_write_config8(cardbus, 0xd3, pci_read_config8(cardbus, 0xd3) & 0x7f);
	pci_write_config32(cardbus, 0x40, 0x60cd17aa);
	pci_write_config8(cardbus, 0xf6, (pci_read_config8(cardbus, 0xf6) & 0xf3) | 0x08);
	pci_write_config8(cardbus, 0xdb, pci_read_config8(cardbus, 0xdb) | 0x40);
	pci_write_config8(cardbus, 0xe8, pci_read_config8(cardbus, 0xe8) & 0xf3);
	pci_write_config8(cardbus, 0x90, pci_read_config8(cardbus, 0x90) | 0x03);
	pci_write_config8(cardbus, 0xf4, pci_read_config8(cardbus, 0xf4) | 0x02);
	pci_write_config8(cardbus, 0x91, pci_read_config8(cardbus, 0x91) | 0x01);
	pci_write_config8(cardbus, 0xdc, pci_read_config8(cardbus, 0xdc) & 0xbf);
	pci_write_config8(cardbus, 0xf1, pci_read_config8(cardbus, 0xf1) | 0x70);
	pci_write_config8(cardbus, 0xf5, (pci_read_config8(cardbus, 0xf5) & 0xc3) | 0x34);
	pci_write_config8(cardbus, 0xd3, pci_read_config8(cardbus, 0xd3) | 0x80);
}

BOOT_STATE_INIT_ENTRY(BS_PAYLOAD_BOOT, BS_ON_ENTRY, configure_cardbus, NULL);
