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
| SD card | **PASS** | The tester confirmed that the card reader works and the SD write test passed. The test log shows a mounted FAT card. Read-back after reinsertion was not separately reported. |
| 32-bit CardBus | **PASS (enumeration and driver)** | O2Micro `1217:8134` with `yenta_cardbus` enumerated an inserted NEC uPD72870 FireWire card (`1033:00cd`). `firewire_ohci` initialized it and Linux created `fw0` at S400. External FireWire transfer and 16-bit PCMCIA remain untested. |
| mSATA | **PASS (read)** | Kingston `SNS4151S316G` detected on `ata3` at 6.0 Gbit/s. A 64 MiB read completed while the NVMe was also present. SATA boot and writes remain untested. |
| M.2 WLAN / Bluetooth | WLAN detected, hardware-blocked | Intel Wireless-AC 3165 (`8086:3166`) loaded `iwlwifi` firmware and exposed a WLAN interface. `rfkill` reported `Hard blocked: yes` on the bare board. Bluetooth HCI was present and unblocked. WLAN association and Bluetooth pairing remain untested. |
| Audio and Ethernet | Drivers bound | `snd_hda_intel` and `e1000e`. Playback and Ethernet traffic remain untested. |
| TPM 2.0 | **FAIL (Linux device unavailable)** | ACPI advertises TPM2. `/dev/tpm0` and `/dev/tpmrm0` were absent after boot. Both systemd device jobs timed out after about 90 seconds. |
| Suspend and internal keyboard | NOT TESTED | Pending |

## NVMe and Intel ME, 2026-11-22

The earlier trimmed ME configuration did not enumerate the NVMe device. Full test-board ME with HAP clear restored enumeration. The latest test with full ME and HAP set also reached the installed Linux system on the NVMe. The earlier reboot loop has no confirmed root cause.

The tested variant uses `descriptor_testmb_hap.bin` derived with `me_cleaner -s` from a full ROM with the original test-board ME. The tested build uses that descriptor and the unchanged `me_testmb.bin` with build-time ME cleaning disabled. The original 16 MiB layout is retained, including ME at `0x3000–0x7fffff` and BIOS at `0x800000–0xffffff`. After building, compare the ME region with the input blob and check HAP using `ifdtool -p sklkbl -d`. The HAP setting is confirmed in the descriptor. The ME runtime state still needs independent validation.

## Remaining firmware tests

- The Secure Boot build uses `SMMSTORE=y` and `EDK2_SECURE_BOOT_SUPPORT=y`, with enforcement off by default in the reference configuration. The ROM has a 512 KiB `SMMSTORE` region at `0x810000`, and its descriptor reports HAP set. The Ubuntu test boot confirmed enforcement. Setup-menu behavior and repeated cold-start persistence were not separately logged.
- The 64 MiB prefetchable CardBus window allocation still fails early. The smaller windows were sufficient for the tested FireWire card's BARs and driver initialization. Test actual FireWire I/O and a 16-bit PCMCIA card separately.
- Repeat WLAN on the assembled VX-2 with the factory wireless switch connected and on. The missing switch is a hypothesis for the bare-board hard block. Verify Bluetooth pairing and traffic separately.
- Check TPM availability on the assembled unit and compare against OEM firmware if still absent. The observed software wait and the underlying TPM detection problem need separate checks.
- Verify USB SuperSpeed per port, suspend/resume, internal input, audio, Ethernet, external displays, and battery/charging on the assembled notebook.
- The reference configuration now disables the persistent SPI flash console (`CONFIG_CONSOLE_SPI_FLASH`). Keep CBMEM logging, SPI flash support, and SMMSTORE enabled. The SPI-console-off build still needs hardware validation.
