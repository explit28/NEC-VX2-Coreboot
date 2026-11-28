# NEC VersaPro VX-2 coreboot port

This repository publishes the NEC VersaPro VX-2 board port and the generic CardBus resource fix currently under review in coreboot Gerrit. The Gerrit changes have **not** yet been merged into coreboot.

## Hardware

The tested system is a NEC VersaPro VX-2 (Compal LA-C423P) with an Intel Core i5-8250U, Intel UHD 620, Nuvoton NPCE388N EC, 16 MiB SPI flash, O2 Micro CardBus/PCMCIA controller (`1217:8134`) and O2 Micro SD reader (`1217:8621`).

The owner reports a complete hardware test of the 16 MiB reference ROM. The reported test includes the dynamically enumerated SD reader, 32-bit CardBus, 16-bit PCMCIA, and the OEM Intel GOP with EDK2. The generic CardBus code programs both base and inclusive limit registers for the two memory and two I/O windows, and disables unused windows. No fixed PCI bus number is required for the SD reader. Individual register dumps, device models, per-port measurements, and the observed SD bus number were not supplied.

See [the hardware test matrix](docs/hardware-test-matrix.md) for the reported scope and known limits. Windows Intel UHD 620 Code 43 remains unresolved. Successful preboot GOP output does not establish working Windows graphics acceleration.

## Gerrit source

- Coreboot base: [`096fd34d9ca95a267802c25ce697498d41243378`](https://review.coreboot.org/plugins/gitiles/coreboot/+/096fd34d9ca95a267802c25ce697498d41243378/)
- Generic CardBus fix, [Gerrit #97774 patchset 2](https://review.coreboot.org/c/coreboot/+/97774): `0ff84145fa0dcf5a6e99fecbc652e9d2b90474a2`
- NEC VersaPro VX-2 port, [Gerrit #97759 patchset 3](https://review.coreboot.org/c/coreboot/+/97759): `c02f5c35dbc5a1d083bae7943be3b649d3877992` (depends on #97774)

The two commits are mirrored byte-for-byte as source files under coreboot's original paths and as mail patches in `patches/`. Apply the patches in order to the listed base revision.

## Build outline

This repository is a patch/source snapshot, not a full coreboot checkout or a ready-to-flash ROM.

1. Check out coreboot at the base revision above.
2. Apply `patches/0001-cardbus.patch`, then `patches/0002-nec-versapro-vx2.patch` with `git am`.
3. Use `config/tested-upstream.config` as a reference (`cp config/tested-upstream.config .config`, then run `make olddefconfig`). The saved test configuration has been sanitized: the notebook serial-number setting was removed and the local payload build path was replaced with `payloads/UEFIPAYLOAD.fd`.
4. Supply the pinned EDK2 sources and payload plus the required coreboot FSP package and notebook-specific firmware files listed in [OEM firmware requirements](docs/oem-firmware.md). Build coreboot using the normal coreboot build process.

The reference configuration selects the Intel OEM GOP/VBT, EDK2 LVGL, and a 16 MiB ROM. It is not a substitute for checking the expanded Kconfig and final image against the target machine before flashing.

## Tested ROM provenance

The owner reports testing a 16,777,216-byte ROM with SHA-256:

`5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`

The ROM is not distributed here. See [build provenance](docs/build-provenance.md) for the test configuration and external inputs.

## Safety

Keep a verified original firmware dump and a recovery plan before flashing. Use this firmware only on the NEC VersaPro VX-2 hardware it targets. Verify the image layout, descriptor, payload and checksum locally. Do not flash the included source snapshot or patches directly.

Earlier experimental IGD and EDK2 investigations remain in the `archive-experimental-20261128` GitHub branch; this branch contains only the current Gerrit source snapshot and supporting reference material.
