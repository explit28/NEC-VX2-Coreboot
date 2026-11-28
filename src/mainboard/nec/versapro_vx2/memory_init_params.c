/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NEC VersaPro VX-2 / LA-C423P
 * Board-specific FSP-M configuration.
 * SPD addresses and channel allocation verified from stock firmware.
 */

#include <soc/romstage.h>
#include <soc/soc_chip.h>
#include <spd_bin.h>

void mainboard_memory_init_params(FSPM_UPD *mupd)
{
	FSP_M_CONFIG *mem_cfg = &mupd->FspmConfig;

	/* Reserve 128 MiB for integrated graphics as required by the test setup. */
	mem_cfg->IgdDvmt50PreAlloc = IGD_SM_128MB;

	/* DDR4 channel configuration from the board's factory setup. */
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
}
