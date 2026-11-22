/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * NEC VersaPro VX-2 / LA-C423P.
 *
 * Early native-function muxing for the NPCE388N LPC connection,
 * based on pads observed under the NEC factory firmware.
 *
 * This does NOT attempt any vendor EC command sequence or change
 * unknown power/reset GPIO outputs.
 */
#include <bootblock_common.h>
#include <soc/gpio.h>

static const struct pad_config early_gpio_table[] = {
	/* EC LPC interface: RCIN#, LAD[0:3], LFRAME#, SERIRQ. */
	PAD_CFG_NF(GPP_A0, NONE, DEEP, NF1),
	PAD_CFG_NF(GPP_A1, NATIVE, DEEP, NF1),
	PAD_CFG_NF(GPP_A2, NATIVE, DEEP, NF1),
	PAD_CFG_NF(GPP_A3, NATIVE, DEEP, NF1),
	PAD_CFG_NF(GPP_A4, NATIVE, DEEP, NF1),
	PAD_CFG_NF(GPP_A5, NONE, DEEP, NF1),
	PAD_CFG_NF(GPP_A6, NONE, DEEP, NF1),

	/* CLKRUN# and two LPC clock outputs. */
	PAD_CFG_NF(GPP_A8, NONE, DEEP, NF1),
	PAD_CFG_NF(GPP_A9, NATIVE, DEEP, NF1),
	PAD_CFG_NF(GPP_A10, NATIVE, DEEP, NF1),

	/* DDR4 SPD SMBus: preserve the observed factory pad settings. */
	_PAD_CFG_STRUCT(GPP_C0, PAD_FUNC(NF1) | PAD_RESET(DEEP) |
		PAD_TRIG(OFF) | PAD_BUF(TX_DISABLE) | (1 << 1), 0),
	_PAD_CFG_STRUCT(GPP_C1, PAD_FUNC(NF1) | PAD_RESET(DEEP) |
		PAD_TRIG(OFF) | PAD_BUF(TX_DISABLE) | (1 << 1), 0),
};

void bootblock_mainboard_early_init(void)
{
	gpio_configure_pads(early_gpio_table, ARRAY_SIZE(early_gpio_table));
}
