# Build snapshot provenance

Generated: 2026-11-22T01:44:55Z

Base revision: `8b0e34f90e08547cf19e550a43ed72f69c39d6ed`

Working tree status at snapshot time:

```text
?? src/mainboard/nec/versapro_vx2/
```

Files copied: board source files and complete `.config` only. OEM binaries excluded.


## Known-good ROM capture, 2026-11-24

- Configuration: extracted directly from `build/coreboot-known-good.rom`; published as `config/coreboot.config` with the device serial number cleared.
- Observed coreboot checkout: `8b0e34f90e08547cf19e550a43ed72f69c39d6ed`.
- Configured EDK2 ref: `26.09.1` (MrChromebox).
- Observed EDK2 checkout: `57642656c3ddc5d55aeb2e8c3fb86c81276da4a7`.
- Current recursive submodule revisions: [submodules.txt](submodules.txt). These were collected from the checkout, not independently recovered from the ROM.
- Board source snapshot synchronized from the original development checkout. Local graphics diagnostics outside the board directory and EDK2 workspace changes have not been captured in this snapshot. Exact binary reproducibility is not established. OEM binaries remain local.

## Local source and configuration sync, 2026-11-25

- Coreboot base revision: `8b0e34f90e08547cf19e550a43ed72f69c39d6ed`.
- The published reference comes from the working tree’s `.config`, with the per-unit serial number cleared. It disables unconditional above-4-GiB PCI allocation and points to local inputs named `descriptor_hap.bin`, `me.bin` and `gbe.bin`.
- The board sources disable FSP PCIe root-port function swapping and include the complete O2 Micro CardBus register sequence at payload boot. The devicetree enables root port 9 and disables the imaging unit.
- The coreboot patch selects memory window 1 for CardBus prefetch and programs the assigned window limits on the VX-2. It moves the SD reader’s root-port secondary bus from `07` to `08` when enumeration would otherwise assign `07`.
