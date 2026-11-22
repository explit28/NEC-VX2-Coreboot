# Test matrix

| Test | Result | Evidence / notes |
|---|---|---|
| Coreboot boot stages reach EDK2 payload | PASS | Persistent SPI console: `Jumping to boot code` |
| libgfxinit eDP AUX / external DDC probing | FAIL | EDID reads failed; no display selected |
| Legacy Intel VGA ROM launch | PASS | `Option ROM returned`, `VGA Option ROM was run` |
| Legacy VESA 1024x768 visible output | FAIL | Backlight on, black screen |
| Legacy VESA 1280x1024 visible output | FAIL | Backlight on, black screen |
| OEM Intel GOP + VBT graphical output | **PASS** | Visible coreboot logo in EDK2 |
| EDK2 UEFI Shell | **PASS** | Shell appeared after splash |
| EDK2 graphical setup (LVGL + NEC OEM GOP) | **PASS** | Graphical Firmware Setup photographed on the test board |
| USB boot into Ubuntu Live | **PASS** | Booted a USB stick after the USB and PCIe bring-up changes. Per-port SuperSpeed operation remains to be verified. |
| USB on all five physical ports | **PASS** | The tester confirmed working USB on all five ports. Negotiated SuperSpeed rates were not recorded per port. |
| Internal NVMe detection and Linux boot with HAP | **PASS** | Kingston NV2 `2646:501d` at `03:00.0` behind `00:1c.4`. Linux 7.0.0-44 mounted its ext4 root on `nvme0n1p2` and remounted it read/write. |
| External HDMI/VGA video with OEM GOP | NOT TESTED | No independently confirmed result |
| UEFI Secure Boot and Ubuntu NVMe boot | **PASS** | `mokutil --sb-state` reported enabled. The kernel confirmed EFI Secure Boot lockdown. Ubuntu booted via `shimx64.efi` with `/` on `nvme0n1p2`. |
| SD card | **PASS** | The tester confirmed operation. The test log also shows a mounted FAT card. Write and read-back were not separately recorded. |
| CardBus socket | Initialized | O2Micro `1217:8134` bound to `yenta_cardbus`. Card insertion and operation remain untested. |
| SATA | Controller detected | AHCI bound and reported three implemented ports. No SATA drive was verified. |
| Audio and Ethernet | Drivers bound | `snd_hda_intel` and `e1000e`. Playback and Ethernet traffic remain untested. |
| TPM 2.0 | Open | ACPI TPM2 and EFI event-log entries are present. IMA reported no TPM chip at initialization. Runtime availability needs a device-node and driver check. |
| Suspend and internal keyboard | NOT TESTED | Pending |

## NVMe and Intel ME, 2026-11-22

The earlier trimmed ME configuration did not enumerate the NVMe device. Full test-board ME with HAP clear restored enumeration. The latest test with full ME and HAP set also reached the installed Linux system on the NVMe. The earlier reboot loop has no confirmed root cause.

The tested variant uses `descriptor_testmb_hap.bin` derived with `me_cleaner -s` from a full ROM with the original test-board ME. The tested build uses that descriptor and the unchanged `me_testmb.bin` with build-time ME cleaning disabled. The original 16 MiB layout is retained, including ME at `0x3000–0x7fffff` and BIOS at `0x800000–0xffffff`. After building, compare the ME region with the input blob and check HAP using `ifdtool -p sklkbl -d`. The HAP setting is confirmed in the descriptor. The ME runtime state still needs independent validation.

## Remaining firmware tests

- The Secure Boot build uses `SMMSTORE=y` and `EDK2_SECURE_BOOT_SUPPORT=y`, with enforcement off by default in the reference configuration. The ROM has a 512 KiB `SMMSTORE` region at `0x810000`, and its descriptor reports HAP set. The Ubuntu test boot confirmed enforcement. Setup-menu behavior and repeated cold-start persistence were not separately logged.
- PCI allocation of a 64 MiB prefetchable CardBus window failed early. The initialized socket subsequently showed two 4 MiB memory windows and two I/O windows. Check actual 32-bit CardBus and 16-bit PCMCIA cards before deciding whether PCI resource changes are needed.
- Verify USB SuperSpeed per port, SATA with a drive installed, and suspend/resume with HAP enabled.
