# NEC VX-2 IGD subsystem ID

| Source | PCI 00:02.0 DWORD 0x2C | Subsystem |
| --- | --- | --- |
| Original NEC BIOS PCI dump | `0x60CF17AA` | `17AA:60CF` |
| EFI diagnostic 00 | `0x22128086` | `8086:2212` |
| EFI diagnostic 01, Combined V2 | `0x221217AA` | `17AA:2212` |
| SID fix target | `0x60CF17AA` | `17AA:60CF` |

Both diagnostics report host bridge GGC `0x04C1`, IGD MSAC `0x01` and
an 8 MiB GGTT. They sample the machine after GOP. The OEM PCI dump
establishes the factory value. The OEM `SiInitPreMem` code provides a
reference for early graphics setup, including GGC and MSAC. The exact OEM
instruction and policy field responsible for the subsystem ID remain untraced.

## Why only the vendor changed

The Combined V2 devicetree and generated `static.c` specify `17AA:60CF`
for the IGD. The two global coreboot subsystem settings are zero.
During ramstage resource enable, the IGD uses `pci_dev_enable_resources()`
and `pci_dev_set_subsystem()` to write `0x60CF17AA` as a DWORD at `0x2C`.
The generic callback does not split the value or derive `2212` from the PCI ID.

The Kaby Lake FSP-M image builds SA policy with `0x2212` in the active
display-selection path at image offsets `0x5C13A–0x5C14B`. The routine
at `0x4760C–0x4762D` reads the policy word, resolves PCI ECAM for bus 0,
device 2, function 0, and ORs that word into the 16-bit register at
`0x2E`. It does not touch the adjacent vendor word. Both fields have
independent write-once behavior. By PCI resource enable, the device word
has already latched `2212` and the vendor word accepts `17AA`.

The FSP-S `DefaultSvid` and `DefaultSid` fields are SA-wide and are set
to zero by coreboot, as are the two PCH subsystem defaults. The earlier
FSP-M store operates independently of these fields. The default EDK II
diagnostic reads the IDs, and the Post-GOP DXE source writes ACPI OpRegion
fields. The FSP-M write explains the observed SID without a later write
by EDK II or the OEM GOP. The available EFI reports do not capture the
transition out of FSP-M.

[Processor graphics register access, sections 4.13 and 4.14](https://cdrdv2-public.intel.com/335191/335191-003-kbl-h-processor-datasheet-vol-2.pdf) · [Intel Kaby Lake FSP policy](https://github.com/intel/FSP/blob/master/KabylakeFspBinPkg/Fsp.bsf)

## Correction and validation

The board memory initialization callback sets `0x60CF` with a single
16-bit write before FSP-M. It requires enabled integrated graphics,
PCI ID `8086:5917` and the SID reset value of zero. It checks the
readback and logs any unexpected value. The previous PreMem experiment
used a DWORD write that also programmed the SVID. The new change leaves
that word to the existing ramstage callback. The reason for the earlier
boot failure is unconfirmed.

The 16 MiB build passes FMAP and Intel descriptor validation. Compared
with Combined V2, the descriptor, ME, GbE, DSDT, FSP-M/S, VBT, legacy
option ROM and EDK II payload compare byte for byte. The rebuilt
romstage contains the guarded 16-bit SID store and the existing
`IgdDvmt50PreAlloc = 0x04` assignment. The SWSCI handlers are present in
AML, and the activation status is unchanged.

Windows Code 43 persists on Combined V2. The SID fix requires a cold-boot
hardware check of the coreboot handoff log and EFI diagnostic readback,
followed by Windows hardware ID `PCI\VEN_8086&DEV_5917&SUBSYS_60CF17AA`.
The result of the Windows graphics driver test remains open.
