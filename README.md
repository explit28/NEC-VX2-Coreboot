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

## Porting reference for ThinkPad L570 and L560

The NEC VX-2 port can serve as a starting point for a separate **ThinkPad L570** port. The L570's mobile Skylake/Kaby Lake platform is closely related, and it also uses DDR4. The **ThinkPad L560** is another candidate for investigation. Its Skylake platform shares some of the same initialization work, and its DDR3L memory requires a separate RAM initialization configuration. The results in this repository are from the NEC VX-2. Both Lenovo models require their own hardware validation.

| Hardware | NEC VersaPro VX-2 (tested unit) | ThinkPad L570 (Lenovo PSREF) | ThinkPad L560 (Lenovo PSREF) |
|---|---|---|---|
| Compal PCB | `LA-C423P` on the tested unit | `LA-C422P` (CILL1/CILL2) | `LA-C421P` (AILL1/AILL2) |
| CPU and integrated graphics | Core i5-8250U (Kaby Lake-R, 4 cores), UHD 620 | Skylake or Kaby Lake variants, HD 520, 610 or 620 | Skylake variants, HD 510 or 520 |
| Memory | DDR4 | Two DDR4 SO-DIMM slots | Two DDR3L SO-DIMM slots |
| External display | VGA and HDMI, functional tests pending | VGA and Mini DisplayPort | VGA and Mini DisplayPort |
| Internal storage | NVMe boot verified with a PCIe adapter in the 2.5-inch drive bay. mSATA reads also verified | 2.5-inch SATA or M.2 PCIe NVMe in the same drive bay with a dedicated adapter. Optional M.2 SATA storage in the WWAN slot on some configurations | 2.5-inch SATA HDD, SSD or SSHD configurations listed |
| Expansion | 32-bit CardBus enumeration verified. Five physical USB ports | Optional ExpressCard/34, four USB ports, ThinkPad dock connector | Optional ExpressCard/34, four USB ports with speeds depending on model, ThinkPad dock connector |
| TPM | OEM firmware exposed TPM 2.0 via Linux `tpm_tis`. Detection under the tested coreboot build is unresolved | Discrete TPM 2.0 | Lenovo lists discrete TPM 1.2 and chipset firmware TPM 2.0 |

Reuse the related Intel SoC initialization and the VX-2 bring-up workflow for memory, graphics and PCIe. For each Lenovo board revision, determine the DIMM/SPD configuration, GPIO and PCIe routing, EC interface and ACPI methods, panel configuration, TPM transport, and SPI layout from that target. The VX-2 WLAN fix is a reference for tracing an EC-controlled radio block. Its MMIO address and EC bit are specific to the tested NEC implementation. Use the target's own firmware dump and board-specific components. **Flash the VX-2 image only on the VX-2.**

### NVMe adapter for the 2.5-inch drive bay

The tested VX-2 boots NVMe through a dedicated caddy connected to the PCIe-capable drive-bay connector. The drive bay also accepts SATA storage. The caddy carries PCIe signals to an M.2 NVMe SSD.

For the ThinkPad L570, Lenovo calls the part the **M.2 solid-state drive adapter in the internal-storage-drive bay**. The [L570 service manual](https://download.lenovo.com/pccbbs/mobiles_pdf/l570_hmm_en_sp40k88826_02.pdf) shows the adapter. [Drive Solutions' L570 parts reference](https://drivesolutions.com/upgrade/model/view/id/440900/) identifies **01AY477** as the M.2 2280 NVMe adapter and **01LW340** as its cable. The **01LW339** cable is for the SATA drive configuration.

The project has confirmed the NVMe caddy approach on the VX-2. The label on the tested NEC caddy has not been matched to the Lenovo FRU, and compatibility of those FRUs with the L560 is not verified.

The schematic indexes for [L570 LA-C422P](https://awesomeheader.com/model/lenovo-thinkpad-l570/) and [L560 LA-C421P](https://eletronicabr.com/en/files/file/44855-lenovo-l560-notebook-electrical-schematic-aill1-aill2-and-la-c421p-rev-10-schematic/) identify different PCBs. Prioritize the L570 for a new port based on the VX-2 work. For the L560, use the VX-2 as a reference for SoC bring-up and derive board initialization from the LA-C421P schematic and target firmware. The PCB identifiers alone do not establish shared routing.

Lenovo references: [ThinkPad L570 platform specifications](https://psref.lenovo.com/syspool/Sys/PDF/ThinkPad/ThinkPad_L570/ThinkPad_L570_Spec.PDF), [ThinkPad L570 model configurations](https://psref.lenovo.com/Product/ThinkPad_L570), [ThinkPad L560 platform specifications](https://psref.lenovo.com/syspool/Sys/PDF/ThinkPad/ThinkPad_L560/ThinkPad_L560_Spec.PDF).

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
