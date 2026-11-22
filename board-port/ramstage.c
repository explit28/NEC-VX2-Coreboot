/* SPDX-License-Identifier: GPL-2.0-only */
/* NEC VersaPro VX-2: pre-FSP-S GPIO hook. */
#include <soc/ramstage.h>
#include "gpio.h"

void mainboard_silicon_init_params(FSP_SIL_UPD *params)
{
	(void)params;
	mainboard_configure_late_gpios();
}
