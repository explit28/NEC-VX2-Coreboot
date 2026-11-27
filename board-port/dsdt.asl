/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NEC VersaPro VX-2 / LA-C423P
 * Experimental ACPI DSDT.
 * Minimal EC is included automatically by Skylake lpc.asl.
 * Battery, fan and hotkeys are not yet implemented.
 */

#include <acpi/acpi.h>

DefinitionBlock(
	"dsdt.aml",
	"DSDT",
	ACPI_DSDT_REV_2,
	OEM_ID,
	ACPI_TABLE_CREATOR,
	COREBOOT_OEM_REVISION
)
{
	#include <acpi/dsdt_top.asl>
	#include <soc/intel/common/block/acpi/acpi/platform.asl>
	#include <soc/intel/common/block/acpi/acpi/globalnvs.asl>
	#include <cpu/intel/common/acpi/cpu.asl>

	Device (\_SB.PCI0)
	{
		#include <soc/intel/skylake/acpi/systemagent.asl>
		#include <soc/intel/skylake/acpi/pch.asl>
		#include <drivers/intel/gma/acpi/default_brightness_levels.asl>
	}

#if CONFIG(NEC_VX2_IGD_OEM_COMPAT)
	#include "acpi/igd-oem.asl"
#else
	Scope (\_SB.PCI0.GFX0)
	{
		/* Intel IGD power and mode notifications supported by the NEC BIOS. */
		Method (_DSM, 4, Serialized)
		{
			If (Arg0 == ToUUID ("3e5b41c6-eb1d-4260-9d15-c71fbadae414") &&
			    Arg1 == One)
			{
				If (Arg2 == Zero)
				{
					Return (Buffer (One) { 0x7D })
				}

				If ((Arg2 >= 0x02) && (Arg2 <= 0x06))
				{
					Return (One)
				}
			}

			Return (Buffer (One) { Zero })
		}
	}

#endif

	#include <southbridge/intel/common/acpi/sleepstates.asl>
}
