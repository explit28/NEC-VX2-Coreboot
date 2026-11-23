# NEC VersaPro VX-2 — coreboot + EDK2 (experimental port)

> Experimental, unofficial port. Keep a verified backup of the original SPI flash and a known-recoverable external programmer setup. Do not flash another device's complete image. The bundled snapshot is not necessarily ready to flash without integrating its board-specific configuration and required OEM components.

## Hardware test status

The results below cover the tested coreboot 26.09 builds on the VX-2. See the [test matrix](docs/test-matrix.md) for the hardware and firmware details of each result.

| Status | Functions |
|---|---|
| **Working on hardware** | Internal display with NEC GOP and VBT, EDK II graphical setup and UEFI Shell, USB live boot and USB devices on all five ports, Ubuntu boot from internal NVMe with UEFI Secure Boot enabled, SD card writes, internal Intel AX200 WLAN on the assembled notebook, and connection with the tested D-Link USB WLAN adapter. |
| **Partially verified** | mSATA detection and a 64 MiB read, 32-bit CardBus enumeration and FireWire driver initialization, Bluetooth HCI detection, and audio and Ethernet driver binding. |
| **Known failure** | Linux did not expose `/dev/tpm0` or `/dev/tpmrm0` in the tested build. The corresponding systemd jobs timed out after about 90 seconds. |
| **Still to test** | External HDMI/VGA, USB SuperSpeed on each port, pre-boot USB keyboard, suspend/resume, internal input, battery and charging, AX200 after resume, AC 3165 with the WLAN fix, Bluetooth pairing and traffic, audio playback, Ethernet traffic, SATA boot and writes, FireWire transfers, 16-bit PCMCIA, and Secure Boot setting persistence across cold starts. |

## Development problems and resolutions

| Problem | Change or finding | Result |
|---|---|---|
| Internal panel stayed black with libgfxinit | eDP AUX and external DDC probes failed to read EDID. Switched to the NEC OEM GOP and matching VBT through EDK II. | **Resolved for the internal display**. External outputs remain untested. |
| Legacy Intel VGA ROM ran but produced no visible image | VBE modes `0x118` and `0x11b` lit the backlight without visible output. The same OEM GOP and VBT path provided the working display. | **Legacy path failed**. The GOP path showed the logo, graphical setup and UEFI Shell. |
| USB boot was unreliable during bring-up | Tested again after adding the board's USB port configuration and PCIe bring-up changes. | **USB live boot confirmed**. USB devices worked on all five ports. |
| Internal NVMe disappeared with trimmed Intel ME | Restored the full original ME. Tested first with HAP clear, then with HAP set and build-time ME cleaning disabled. | **NVMe boot confirmed with full ME and HAP set**. The earlier reboot loop has no confirmed root cause. |
| UEFI Secure Boot support needed a variable store | Enabled SMMSTORE and MrChromebox EDK II Secure Boot support. | **Enforcement confirmed** by Ubuntu booting from NVMe through shim with kernel lockdown active. |
| Internal WLAN reported a hardware RF-kill block | The OEM GPD9 pad setting alone left the block in place. The fix opens the LPC MMIO window and sets the OEM EC `DCWL` bit during ACPI initialization. | **AX200 confirmed working** on the assembled VX-2. The bare-board AC 3165 needs a retest. |
| CardBus prefetchable window allocation failed | The smaller windows were sufficient for the tested FireWire card to enumerate and initialize its driver. | **Partial**. The 64 MiB allocation failure remains open, along with transfer testing. |
| TPM 2.0 devices were absent in Linux | The firmware advertises an ACPI TPM2 table. Linux still reported the two device jobs timing out. | **Open**. No firmware fix has been verified. |

## Hardware / software

- System: NEC VersaPro VX-2, Compal LA-C423P (board identification from porting work)
- CPU: Intel Core i5-8250U (Kaby Lake-R)
- iGPU: Intel UHD Graphics 620, PCI ID `8086:5917`
- EC: Nuvoton NPCE388N
- SPI flash: Winbond W25Q128BV (SOIC-8), 16 MiB. Tested with an external T48 programmer. The `minipro` device name is `W25Q128BV@SOIC8`.
- Firmware: coreboot build based on the user's coreboot 26.09 development tree, EDK2 MrChromebox payload

## Build configuration

The reference configuration enables LVGL and uses the original 16 MiB SPI layout with an 8 MiB BIOS region. It incorporates the verified HAP descriptor path, full ME input, SMMSTORE, and EDK II Secure Boot support. Secure Boot starts disabled by default and can be enabled in setup. The reference was updated from confirmed settings in the test build. A fresh full `.config` export of that build is still pending. The reference disables persistent SPI console logging and keeps CBMEM logging and SMMSTORE enabled.

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

EDK2 uses coreboot’s bundled logo (`Documentation/coreboot_logo.bmp`) in the reference configuration.

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

The captured config selects EDK2 ref `26.09.1` and configures memory-mapped TPM 2.0. The latest log exposes the ACPI TPM2 table, but Linux TPM device detection remains unresolved. The board directory was untracked in the source checkout. Other local changes and exact submodule revisions were not captured. This archive has not been independently rebuilt.
