# Test matrix

## Confirmed and partially verified functions

| Test | Result | Evidence / notes |
|---|---|---|
| Coreboot boot stages reach EDK2 payload | ✅ Working | Persistent SPI console: `Jumping to boot code` |
| OEM Intel GOP + VBT graphical output | ✅ Working | Visible coreboot logo in EDK2 |
| EDK2 UEFI Shell | ✅ Working | Shell appeared after splash |
| EDK2 graphical setup (LVGL + NEC OEM GOP) | ✅ Working | Graphical Firmware Setup photographed on the test board |
| USB boot into Ubuntu Live | ✅ Working | Booted a USB stick after the USB and PCIe bring-up changes. |
| USB 2.0 | ✅ Working | USB devices worked on all five physical ports. |
| USB SuperSpeed | ✅ Working | SuperSpeed operation confirmed on the VX-2. |
| Pre-boot USB keyboard | ✅ Working | USB keyboard input works before the operating system boots. |
| Internal NVMe detection and Linux boot with HAP | ✅ Working | Kingston NV2 `2646:501d` at `03:00.0` behind `00:1c.4`. Linux 7.0.0-44 mounted its ext4 root on `nvme0n1p2` and remounted it read/write. |
| UEFI Secure Boot and Ubuntu NVMe boot | ✅ Working | `mokutil --sb-state` reported enabled. The kernel confirmed EFI Secure Boot lockdown. Ubuntu booted via `shimx64.efi` with `/` on `nvme0n1p2`. |
| Windows 11 with Secure Boot | ✅ Working | The tester confirmed Windows 11 boots on the VX-2 with coreboot and UEFI Secure Boot enabled. |
| SD card | ⚠️ Linux only | The SD write test passed under Linux. The reader does not work under Windows. |
| 32-bit CardBus | 🟡 Partially verified | O2Micro `1217:8134` with `yenta_cardbus` enumerated an inserted NEC uPD72870 FireWire card (`1033:00cd`). `firewire_ohci` initialized it and Linux created `fw0` at S400. External FireWire transfer and 16-bit PCMCIA remain untested. |
| SATA | ✅ Working | Confirmed on the VX-2 by the tester. |
| mSATA | ✅ Working | Confirmed by the tester. Earlier logs recorded a 64 MiB read from the Kingston `SNS4151S316G` at 6.0 Gbit/s. |
| SATA optical drive | ✅ Working | Confirmed on the VX-2 by the tester. |
| Internal WLAN | ✅ Working | Tested with a retrofitted Intel AX200. The VX-2 typically shipped with an Intel 8xxx-series WLAN card. |
| Internal WLAN after resume | ✅ Working | WLAN works after resuming from suspend with the retrofitted Intel AX200. |
| Bluetooth | ✅ Working | Bluetooth functionality confirmed on the VX-2. |
| Sound | ✅ Working | The tester confirmed audio playback. |
| RJ45 Ethernet | ✅ Working | Wired Ethernet functionality confirmed on the VX-2. |
| TPM 2.0 under Linux | ✅ Detected | `tpm_tis` detected TPM 2.0 via `MSFT0101:00`. Linux created `/dev/tpm0` and `/dev/tpmrm0`. TPM command and measured boot validation are pending. |
| External VGA | ✅ Working | The tester confirmed video output on the VX-2. |
| Suspend/resume | ✅ Working | The VX-2 suspends and resumes successfully. |
| Internal input | ✅ Working | Internal input works on the VX-2. |
| Battery operation | ✅ Working | The VX-2 operates on battery power. |
| Battery charging | ✅ Working | The battery charges. |

## Known failures

| Function | Status | Observed result |
|---|---|---|
| External HDMI | ❌ No video | Linux reports `HDMI-A-1` connected and reads a 256-byte EDID after the DDPB GPIO change. The monitor still shows no image. |
| 64 MiB prefetchable CardBus window | ❌ Fails | Allocation failed. Smaller windows were sufficient for the tested FireWire card's BARs and driver initialization. |

See the [development problem table](../README.md#development-problems-and-resolutions) for the failed graphics approaches and the changes that restored functionality.

## NVMe and Intel ME, 2026-11-22

The earlier trimmed ME configuration did not enumerate the NVMe device. Full test-board ME with HAP clear restored enumeration. The latest test with full ME and HAP set also reached the installed Linux system on the NVMe. The earlier reboot loop has no confirmed root cause.

The tested variant uses `descriptor_testmb_hap.bin` derived with `me_cleaner -s` from a full ROM with the original test-board ME. The tested build uses that descriptor and the unchanged `me_testmb.bin` with build-time ME cleaning disabled. The original 16 MiB layout is retained, including ME at `0x3000–0x7fffff` and BIOS at `0x800000–0xffffff`. After building, compare the ME region with the input blob and check HAP using `ifdtool -p sklkbl -d`. The HAP setting is confirmed in the descriptor. The ME runtime state still needs independent validation.

## Internal WLAN RF-kill, 2026-11-23

Before the fix, Linux loaded the AX200 firmware and reported a hardware radio block. The port-based EC read at offset `0x3a` returned `0x00`, and the PCH LPC Generic Memory Range register at PCI offset `0x98` read `0x00000000`. Applying the OEM GPD9 pad configuration alone did not clear the block.

The OEM DSDT writes the EC field `DCWL` at MMIO address `0xfe80013a`, bit 5. The tested board code enables the 64 KiB LPC window at `0xfe800000` in bootblock and sets that bit in the EC ACPI `_INI` method while preserving the other bits. The method skips the write if the read returns `0xff`. It uses the OEM enable path without the `WLAC` and `ELNK` policy branches or the OEM `_Q3B` event handler.

The tester confirmed that internal WLAN works with the retrofitted AX200 after flashing the modified BIOS region. WLAN also works after resume. The individual effects of the two firmware changes and the electrical routing of `W_DISABLE1#` were not tested separately.

## 🟡 Remaining firmware tests

- The Secure Boot build uses `SMMSTORE=y` and `EDK2_SECURE_BOOT_SUPPORT=y`, with enforcement off by default in the reference configuration. The ROM has a 512 KiB `SMMSTORE` region at `0x810000`, and its descriptor reports HAP set. The Ubuntu test boot confirmed enforcement. Setup-menu behavior and repeated cold-start persistence were not separately logged.
- Test actual FireWire I/O and a 16-bit PCMCIA card separately.
- Test Bluetooth pairing separately.
- Validate TPM command execution and measured boot separately. The detection test used `systemd.tpm2_wait=0`.
- Record the negotiated SuperSpeed rate for each port.
- The reference configuration now disables the persistent SPI flash console (`CONFIG_CONSOLE_SPI_FLASH`). Keep CBMEM logging, SPI flash support, and SMMSTORE enabled. The SPI-console-off build still needs hardware validation.
