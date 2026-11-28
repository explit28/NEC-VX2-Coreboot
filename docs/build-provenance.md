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

## Current upstream review, 2026-11-27

The maintainer reports the new consolidated mainboard review as
[coreboot Gerrit change #97759](https://review.coreboot.org/c/coreboot/+/97759).
Use this link for the current review instead of the historical four-change stack below.
The current patchset number, exact source revision, mergeability, CI status,
and abandonment status of the preceding changes have not been independently
verified in this documentation update. The source snapshot on GitHub has not
been compared byte-for-byte with the new Gerrit patchset.

## Historical upstream review stack, 2026-11-27

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

### Gerrit mergeability and CI

Coreboot's Gerrit project uses `CHERRY_PICK`. Its mergeability check
tests each patchset against `main` individually. The base board patch
passes that check. The three follow-up changes require board files
introduced by their predecessors, so Gerrit reports them as merge
conflicts while those files are absent from `main`. All four patchsets
apply in the order above. Gerrit's related-changes API shows that exact
parent chain, and its submitted-together API returns an empty list.

The previous Jenkins runs failed the lowercase-`coreboot` lint check
on two GitHub URLs and, in the experimental graphics change, on the
Kconfig help text. The patchsets above contain the spelling fixes.
Gerrit initially copied the old `Verified -1` votes to 97744 and 97745
because their patch content was unchanged. The replacement Jenkins jobs
all passed the previously failing lint test.

All four commits also passed local stable lint, extended lint and
checkpatch and produced a VX-2 coreboot ROM with `PAYLOAD_NONE`.
Those builds validate the default board configuration without an
EDK2 payload or OEM flash layout.

The experimental 97746 configuration also compiled its ramstage and
DSDT with OEM compatibility and integer DSM query enabled. This
check covered the coreboot code and AML, without building EDK2.

At the 2026-11-27 status check, 97596 remained open with Code-Review
and Verified outstanding. The succeeding Gerrit changes were still
reported as individually non-mergeable. The replacement Jenkins jobs
were still running after passing the lowercase check:
[97596](https://qa.coreboot.org/job/coreboot-gerrit/310212/),
[97744](https://qa.coreboot.org/job/coreboot-gerrit/310213/),
[97745](https://qa.coreboot.org/job/coreboot-gerrit/310214/), and
[97746](https://qa.coreboot.org/job/coreboot-gerrit/310215/).
## Final upstream ROM hardware test — 2026-11-28

- User reports complete hardware validation of the final upstream ROM. SHA-256: `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`; size 16 MiB. The binary itself is not published.
- Coreboot series: base `096fd34d9ca95a267802c25ce697498d41243378`; CardBus `94fe8fa9504b97b0bebedbe0d20827d86e886526`, Change-ID `I9a9247b7294720b599f0af9e9f421c1186163093`; NEC port `7ff147979f5e2e7b4d9245e8553f37a596d260ae`, Change-ID `Ic47966b6d08122709f3d73ad5c3634956e62dcc8`, existing Gerrit change [#97759](https://review.coreboot.org/c/coreboot/+/97759).
- Coreboot release string: `26.09`. EDK2 MrChromebox revision: `44b9aa144a5ff5ceb843245388fb00c87a06895f`; `edk2-platforms`: `4db2845d14425599238be86b8a73d288f7c0b4a6`; Kaby Lake FSP: `3.7.6.0`.
- OEM GOP and matching VBT, full notebook ME, GbE and descriptor were used locally. None of these proprietary binary inputs, nor the complete ROM, are included here.
- The final test specifically covered SD with dynamic bus allocation, generic CardBus window handling, 32-bit CardBus, 16-bit PCMCIA, OEM GOP/EDK2 and previously supported hardware. Exact bus number, card models and per-port measurements were not supplied.
- Windows Intel UHD 620 Code 43 remains a known limitation. Roda RK9 was not separately built or tested.
- Source changes are tracked by the [generic CardBus Gerrit change](https://review.coreboot.org/q/I9a9247b7294720b599f0af9e9f421c1186163093) and the existing [NEC mainboard change #97759](https://review.coreboot.org/c/coreboot/+/97759).
