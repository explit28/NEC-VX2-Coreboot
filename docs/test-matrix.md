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
| External USB keyboard in EDK2 | NOT TESTED | Pending |
| Internal NVMe detection and Linux boot with HAP | **PASS** | Kingston NV2 `2646:501d` at `03:00.0` behind `00:1c.4`. Linux 7.0.0-44 mounted its ext4 root on `nvme0n1p2` and remounted it read/write. |
| External HDMI/VGA video with OEM GOP | NOT TESTED | No independently confirmed result |
| Suspend, audio, internal keyboard, TPM, networking | NOT TESTED | Pending |

## NVMe and Intel ME, 2026-11-22

The earlier trimmed ME configuration did not enumerate the NVMe device. Full test-board ME with HAP clear restored enumeration. The latest test with full ME and HAP set also reached the installed Linux system on the NVMe. The earlier reboot loop has no confirmed root cause.

The tested variant uses `descriptor_testmb_hap.bin` derived with `me_cleaner -s` from a full ROM with the original test-board ME. The next build uses that descriptor and the unchanged `me_testmb.bin` with build-time ME cleaning disabled. The original 16 MiB layout is retained, including ME at `0x3000–0x7fffff` and BIOS at `0x800000–0xffffff`. After building, compare the ME region with the input blob and check HAP using `ifdtool -p sklkbl -d`. The HAP setting is confirmed in the descriptor. The ME runtime state still needs independent validation.

## Remaining firmware tests

- UEFI Secure Boot was disabled in the boot log, and the test user reported that the EDK II setup had no Secure Boot option. The repository reference config has SMMSTORE disabled. Check the current build's `SMMSTORE` and `EDK2_SECURE_BOOT_SUPPORT` settings. For a candidate build, verify the `SMMSTORE` FMAP region and keep `EDK2_SECURE_BOOT_DEFAULT_ENABLE` off until setup visibility, variable persistence, and the installed bootloader have been checked.
- CardBus enumerates, but Linux reported failed allocation of a 64 MiB bridge window. Test with an actual card and compare PCI resources with OEM firmware.
- Verify USB SuperSpeed per port, SATA with a drive installed, and suspend/resume with HAP enabled.
