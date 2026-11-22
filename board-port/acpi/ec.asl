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
	Name (_CRS, ResourceTemplate ()
	{
		IO (Decode16, 0x62, 0x62, 0x01, 0x01)
		IO (Decode16, 0x66, 0x66, 0x01, 0x01)
	})
}
