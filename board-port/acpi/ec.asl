/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * NEC stock ECDT (TP-N28): EC at \_SB.PCI0.LPCB.EC,
 * command/status port 0x66, data port 0x62, GPE 0x50.
 * EC fields, battery, thermal and vendor SMI methods are not yet ported.
 * Included automatically by src/soc/intel/skylake/acpi/lpc.asl.
 */
Device (EC)
{
	Name (_HID, EisaId ("PNP0C09"))
	Name (_UID, 0)
	Name (_GPE, 0x50)

#if CONFIG(NEC_VX2_IGD_OEM_COMPAT)
	Name (NERC, Zero)
	OperationRegion (NERM, EmbeddedControl, 0x46, One)
	Field (NERM, ByteAcc, NoLock, Preserve)
	{
		, 2,
		NHPL, 1
	}

	Method (_REG, 2, NotSerialized)
	{
		If (Arg0 == 3)
		{
			NERC = Arg1
		}
	}
#endif

	/* Experimental WLAN initialization based on the OEM DCWL field. */
	OperationRegion (WLER, SystemMemory, 0xFE80013A, 1)
	Field (WLER, ByteAcc, NoLock, Preserve)
	{
		WLRG, 8
	}

	Method (_INI, 0, Serialized)
	{
		Store (WLRG, Local0)
		If (LNotEqual (Local0, 0xFF))
		{
			Or (Local0, 0x20, WLRG)
		}
	}
	Name (_CRS, ResourceTemplate ()
	{
		IO (Decode16, 0x62, 0x62, 0x01, 0x01)
		IO (Decode16, 0x66, 0x66, 0x01, 0x01)
	})
}
