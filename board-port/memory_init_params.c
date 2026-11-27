/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NEC VersaPro VX-2 / LA-C423P
 * Experimental FSP-M configuration.
 * SPD addresses and channel allocation verified from stock firmware.
 */

#include <console/console.h>
#include <device/pci_ops.h>
#include <soc/pci_devs.h>
#include <soc/romstage.h>
#include <spd_bin.h>

static void set_igd_subsystem_device(void)
{
	u16 sid;

	if (pci_read_config32(SA_DEV_IGD, PCI_VENDOR_ID) != 0x59178086)
		return;

	/*
	 * FSP-M programs the write-once IGD SID to 0x2212. Set only the SID
	 * while it is at its reset value. The devicetree supplies SVID later.
	 */
	sid = pci_read_config16(SA_DEV_IGD, PCI_SUBSYSTEM_ID);
	if (sid == 0) {
		pci_write_config16(SA_DEV_IGD, PCI_SUBSYSTEM_ID, 0x60cf);
		sid = pci_read_config16(SA_DEV_IGD, PCI_SUBSYSTEM_ID);
	}
	if (sid != 0x60cf)
		printk(BIOS_WARNING, "NEC IGD: unexpected pre-FSP-M SID %04x\n", sid);
}

void mainboard_memory_init_params(FSPM_UPD *mupd)
{
	FSP_M_CONFIG *mem_cfg = &mupd->FspmConfig;

	/* Keep the experimental 128 MiB graphics preallocation. */
	mem_cfg->IgdDvmt50PreAlloc = 0x04;

	/* Provisional values based on the ThinkPad T580. */
	mem_cfg->DqPinsInterleaved = true;
	mem_cfg->CaVrefConfig = 2;
	mem_cfg->MemorySpdDataLen = CONFIG_DIMM_SPD_SIZE;

	/* Factory DMI and SPD serial numbers: channel A 0x50, channel B 0x52. */
	struct spd_block blk = {
		.addr_map = { 0x50, 0x52 }
	};

	get_spd_smbus(&blk);
	dump_spd_info(&blk);

	mem_cfg->MemorySpdPtr00 = (uintptr_t)blk.spd_array[0];
	mem_cfg->MemorySpdPtr10 = (uintptr_t)blk.spd_array[1];

	if (mem_cfg->InternalGfx)
		set_igd_subsystem_device();
}
