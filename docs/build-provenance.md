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

## Upstream review, 2026-11-27

The coreboot review stack starts at upstream commit
`e91a4d69e6dab247a78ae15e85d85d103f4a800b`. Each patchset uses the
preceding row as its parent. The last Gerrit ref contains the whole stack.

| Change | Patchset | Gerrit ref | Scope |
|---|---:|---|---|
| [97596](https://review.coreboot.org/c/coreboot/+/97596/4) | 4 | `refs/changes/96/97596/4` | Base board port and board documentation |
| [97744](https://review.coreboot.org/c/coreboot/+/97744/2) | 2 | `refs/changes/44/97744/2` | SD bus numbering and O2 Micro CardBus/PCMCIA support |
| [97745](https://review.coreboot.org/c/coreboot/+/97745/2) | 2 | `refs/changes/45/97745/2` | DVMT 128 MiB and OEM IGD subsystem ID `17aa:60cf` |
| [97746](https://review.coreboot.org/c/coreboot/+/97746/2) | 2 | `refs/changes/46/97746/2` | Experimental IGD ACPI methods and OpRegion NVS reservation |

To fetch the exact review stack in a coreboot clone:

```sh
git fetch https://review.coreboot.org/coreboot.git refs/changes/46/97746/2
git log --oneline --reverse e91a4d69e6dab247a78ae15e85d85d103f4a800b..FETCH_HEAD
```

The review adds `board_info.txt` and coreboot board documentation.
The GPIO reference header stays in the development snapshot. The review
requires `CONFIG_NEC_VX2_IGD_OEM_COMPAT` for the board's experimental
graphics DSM methods and omits the development DSDT's fallback DSM.
SWSCI stays disabled in both trees. The review also omits the diagnostic
changes to coreboot's graphics init, libgfxinit, and the separate EDK2
payload modifications.

The [Combined V2 build instructions](combined-v2-build.md) still describe
the hardware-tested development sources based on coreboot 26.09. The
[test matrix](test-matrix.md) and
[Windows graphics results](windows-graphics.md) record observations from
the development firmware. Building the review stack requires separate
validation before those results can be attributed to it.
