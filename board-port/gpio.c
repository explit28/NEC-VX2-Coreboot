/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NEC VersaPro VX-2: conservative post-memory display pad configuration.
 * GPP_E17 matches the original NEC pad snapshot. That snapshot does not
 * establish an electrical power-on sequence for unknown GPIO outputs.
 */
#include <soc/gpio.h>
#include "gpio.h"

static const struct pad_config nec_late_gpio_table[] = {
	/* SLP_WLAN#: match the NEC factory pad configuration. */
	_PAD_CFG_STRUCT(GPD9, PAD_FUNC(NF1) | PAD_TRIG(OFF) |
		PAD_BUF(RX_DISABLE), 0),

	/* OEM NEC native-function display pads; T480/T480s/T580 use same pads. */
	/* DDPB_HPD */
	_PAD_CFG_STRUCT(GPP_E13, 0x84000500, 0x00000025),
	/* DDPC_HPD */
	_PAD_CFG_STRUCT(GPP_E14, 0x84000500, 0x00000026),
	/* OEM DDPB clock and data pads. */
	_PAD_CFG_STRUCT(GPP_E18, 0x44000702, 0),
	_PAD_CFG_STRUCT(GPP_E19, 0x44000702, PAD_PULL(DN_20K)),
	/* DDPC_CTRLCLK */
	_PAD_CFG_STRUCT(GPP_E20, 0x44000702, 0x0000002c),
	/* DDPC_CTRLDATA */
	_PAD_CFG_STRUCT(GPP_E21, 0x44000702, 0x0000102d),

	/* EDP_HPD: original DW0 0x84000502, DW1 0x00000000 */
	_PAD_CFG_STRUCT(GPP_E17, PAD_FUNC(NF1) | PAD_RESET(PLTRST) |
		PAD_TRIG(OFF) | PAD_BUF(TX_DISABLE) | (1 << 1), 0),
};

void mainboard_configure_late_gpios(void)
{
	gpio_configure_pads(nec_late_gpio_table,
			    ARRAY_SIZE(nec_late_gpio_table));
}
