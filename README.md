# NEC VersaPro VX-2 — coreboot + EDK2 (experimental port)

**Status (2026-11-23): graphical EDK II setup, USB live boot, and Ubuntu boot from internal NVMe with UEFI Secure Boot confirmed on hardware.** The tested ROM used full original ME firmware and HAP in the flash descriptor. All five USB ports and SD writes worked. The mSATA read and a 32-bit CardBus FireWire card's enumeration and driver initialization also passed. WLAN was detected but hardware-blocked on the bare board. See the [test matrix](docs/test-matrix.md) for verified results and open checks.

> Experimental, unofficial port. Keep a verified backup of the original SPI flash and a known-recoverable external programmer setup. Do not flash another device's complete image. The bundled snapshot is not necessarily ready to flash without integrating its board-specific configuration and required OEM components.

## Hardware / software

- System: NEC VersaPro VX-2, Compal LA-C423P (board identification from porting work)
- CPU: Intel Core i5-8250U (Kaby Lake-R)
- iGPU: Intel UHD Graphics 620, PCI ID `8086:5917`
- EC: Nuvoton NPCE388N
- SPI firmware: 16 MiB image, external T48 programmer used during testing
- Firmware: coreboot build based on the user's coreboot 26.09 development tree, EDK2 MrChromebox payload

## Milestone: graphical UEFI output

The NEC OEM GOP also renders the LVGL setup menu. A subsequent build booted Ubuntu Live from USB after the USB and PCIe bring-up changes. Per-port SuperSpeed and pre-boot external keyboard input still need testing.

The reference configuration enables LVGL and uses the original 16 MiB SPI layout with an 8 MiB BIOS region. It incorporates the verified HAP descriptor path, full ME input, SMMSTORE, and EDK II Secure Boot support. Secure Boot starts disabled by default and can be enabled in setup. The reference was updated from confirmed settings in the test build. A fresh full `.config` export of that build is still pending. The reference disables persistent SPI console logging for the next assembled-notebook test and keeps CBMEM logging and SMMSTORE enabled.

The following combination was confirmed to show the **coreboot logo through EDK2**, followed by a functional UEFI Shell:

```text
CONFIG_NO_GFX_INIT=y
# CONFIG_VGA_ROM_RUN is not set
CONFIG_INTEL_GMA_ADD_VBT=y
CONFIG_EDK2_REPO_MRCHROMEBOX=y
CONFIG_EDK2_GOP_DRIVER=y
CONFIG_EDK2_USE_LVGL_RENDERER=y
CONFIG_EDK2_GOP_FILE="3rdparty/blobs/mainboard/nec/vx-2/IntelGopDriver.efi"
CONFIG_INTEL_GMA_VBT_FILE="3rdparty/blobs/mainboard/nec/vx-2/IntelGopVbt.bin"
```

These settings should be understood as a **verified subset**, not a complete reproducible `.config`; the captured configuration is included in `config/coreboot.config` with the device serial number cleared.

### Why an original NEC GOP?

Previous attempts failed to produce a visible image:

- libgfxinit: no display EDID from eDP AUX or external DDC; screen stayed blank.
- Legacy x86 Intel option ROM (`pci8086,5917.rom`): ROM executed and VBE modes `0x118` (1024×768×32) and `0x11b` (1280×1024×32) were attempted; the panel backlight lit but no image appeared.
- **NEC OEM UEFI GOP DXE + matching GOP VBT in EDK2: working graphical output.**

The GOP path works for the tested display. External video outputs, suspend, TPM, and audio still need hardware validation.

EDK2 uses coreboot’s bundled logo (`Documentation/coreboot_logo.bmp`) by default. The reference `.config` records the default path. The build uses it automatically, so no manual splash setting or logo extraction is required.

## Firmware component provenance

The original OEM firmware report contained:

| Module | UEFI file GUID | Extracted payload |
|---|---|---|
| IntelGopDriver | `FF0C8745-3270-4439-B74F-3E45F8C77064` | x86-64 PE32+ EFI boot-service driver, 69,568 bytes |
| IntelGopVbt | `878AC2CC-5343-46F2-B563-51F89DAF56BA` | raw VBT, 4,608 bytes; starts with `$VBT SKYLAKE` |

The working VBT had SHA-256 `407ceb6c1a3eafafc9aa441207bc23d69728465ebc046997d300b7052da2ad33`.

**Do not publish OEM binaries, full SPI dumps, ME/BIOS regions or device-specific NVRAM unless you have verified redistribution rights and removed confidential/device-specific data.** This repository is configured to exclude them by default. See [OEM asset instructions](docs/oem-assets.md).

## Restore and reproduce the working development snapshot

1. Use a compatible, separately obtained coreboot source checkout and its required toolchain and submodules.
2. Restore `config/coreboot.config` to `.config` if present.
3. Copy the source files from `board-port/` to the corresponding `src/mainboard/nec/versapro_vx2/` path. Inspect diffs before overwriting.
4. Obtain the **matching OEM GOP + VBT from your own NEC firmware**, place them in the paths described in `docs/oem-assets.md`.
5. Run `make olddefconfig`, then verify `.config` has not reset the graphics settings.
6. Build with `make -j"$(nproc)"` and inspect the complete log for errors.
7. Flash only using a proven recovery workflow and only after confirming the target chip size/partition layout.

See [Development notes](docs/development-notes.md), [Test matrix](docs/test-matrix.md) and [Snapshot script](scripts/snapshot-local.sh).

## License and acknowledgments

Documentation and original repository helper scripts can be released under a permissive license by the author. Coreboot, EDK2, libgfxinit and OEM components retain their respective licenses. This project is **not affiliated with NEC, Intel, Coreboot or MrChromebox**. Do not assume license rights to redistribute extracted OEM firmware.

The captured config selects EDK2 ref `26.09.1` and configures memory-mapped TPM 2.0. The latest log exposes the ACPI TPM2 table, but TPM availability in Linux remains open. The board directory was untracked in the source checkout. Other local changes and exact submodule revisions were not captured. This archive has not been independently rebuilt.
