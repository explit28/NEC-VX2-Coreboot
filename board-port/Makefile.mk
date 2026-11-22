## SPDX-License-Identifier: GPL-2.0-only
# NEC VersaPro VX-2 / LA-C423P board-specific stages.
# gpio_reference.h is a measured stock-firmware snapshot, not a programming table.

bootblock-y += bootblock.c
romstage-y += memory_init_params.c
ramstage-y += gpio.c
ramstage-y += ramstage.c
ramstage-$(CONFIG_MAINBOARD_USE_LIBGFXINIT) += gma-mainboard.ads
