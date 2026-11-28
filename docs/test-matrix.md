# Test matrix

## Tested upstream review update — 2026-11-28

The full NEC VX-2 notebook hardware test was reported completed on the cleaned-up upstream test ROM. The tester reports continued support for the dynamically enumerated SD reader, 32-bit CardBus, 16-bit PCMCIA, OEM GOP/EDK2 and previously supported hardware. Individual measurements, exact SD bus number and card models were not supplied for this run. Windows Intel UHD 620 Code 43 remains unresolved; previously open HDMI and other unverified tests are not reclassified.

- [Gerrit #97774](https://review.coreboot.org/c/coreboot/+/97774): generic CardBus bridge window limit programming, commit `9acf4b12a5e9ee5f1b96c64e27deeb23a24f3b70`.
- [Gerrit #97759](https://review.coreboot.org/c/coreboot/+/97759): updated NEC board port, commit `9430870094bdd7754ea89c05441d7b8c54db2621`, dependent on #97774.
- Both uploads succeeded according to the Git push output. Review and new CI approval remain pending; the previous Jenkins Verified+1 was removed as outdated.
- The generic CardBus fix programs base and inclusive limit registers for both memory and both I/O windows, closes invalid/unassigned windows, and retains Memory Window 0 prefetch. The board-local O2Micro initialization remains. Roda RK9's TI CardBus path is also affected but was not separately built or hardware-tested.
- The SD reader uses dynamic PCI bus allocation; the OEM snapshot locates it behind 00:1d.3. Historical physical root-port mapping ambiguities remain; no fixed bus 08 workaround is retained.
- Experimental IGD NVS, DSM/SWSCI, pre-FSP subsystem-ID override and NEC-specific common graphics debug code were removed from the upstream submission, not from the experimental development snapshot.

Test ROM: 16,777,216 bytes, SHA-256 `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`. Build report: coreboot 26.09, FSP 3.7.6.0, EDK2 `44b9aa144a5ff5ceb843245388fb00c87a06895f`. Tested source commits: CardBus `94fe8fa9504b97b0bebedbe0d20827d86e886526`, NEC `3e34346260337fd245533e47a40da18ffd45cf12`. Later NEC documentation-only change and commit-message corrections produced the submitted commits. The final message-only corrections preserved both tree hashes. Reported IFD/GbE/full ME match the notebook blobs; IFD/FMAP/CBFS validation and local lint/checkpatch passed. The ROM and build artifacts have not been independently inspected for this documentation update.

The GitHub source/configuration snapshot remains the experimental development version, not a byte-for-byte copy of the tested upstream source. Use the Gerrit changes for the upstream code. No ROM or proprietary blobs are published by this update.

## Confirmed and partially verified functions

| Test | Result | Evidence / notes |
|---|---|---|
| Coreboot boot stages reach EDK2 payload | ✅ Working | Persistent SPI console: `Jumping to boot code` |
| OEM Intel GOP + VBT graphical output | ✅ Working | Visible coreboot logo in EDK2 |
| EDK2 UEFI Shell | ✅ Working | Shell appeared after splash |
| EDK2 graphical setup (LVGL + NEC OEM GOP) | ✅ Working | Graphical Firmware Setup photographed on the test board |
| Intel graphics under Linux | ✅ Working (OEM GOP) | Intel UHD 620 with i915; OpenGL 4.6 hardware acceleration confirmed. |
| USB boot into Ubuntu Live | ✅ Working | Booted a USB stick after the USB and PCIe bring-up changes. |
| USB 2.0 | ✅ Working | USB devices worked on all five physical ports. |
| USB SuperSpeed | ✅ Working | SuperSpeed operation confirmed on the VX-2. |
| Pre-boot USB keyboard | ✅ Working | USB keyboard input works before the operating system boots. |
| Internal NVMe detection and Linux boot with HAP | ✅ Working | Kingston NV2 `2646:501d` at `03:00.0` behind `00:1c.4`. Linux 7.0.0-44 mounted its ext4 root on `nvme0n1p2` and remounted it read/write. |
| UEFI Secure Boot and Ubuntu NVMe boot | ✅ Working | `mokutil --sb-state` reported enabled. The kernel confirmed EFI Secure Boot lockdown. Ubuntu booted via `shimx64.efi` with `/` on `nvme0n1p2`. |
| Windows 11 with Secure Boot | ✅ Working | Previously verified Windows 11 boot with Secure Boot enabled; this is not a native graphics acceleration test. |
| Secure Boot persistence | ✅ Working | Secure Boot settings persist across restarts. |
| SD card (early development build) | ✅ Linux: Working<br>✅ Windows: Working | The earlier development test used a fixed bus-08 workaround. The final upstream ROM was tested with dynamic assignment; its exact bus number was not reported. |
| 32-bit CardBus | ✅ Linux: Working<br>✅ Windows: Working | FireWire and USB 2.0 CardBus cards tested. Data transfer succeeded on both operating systems. |
| 16-bit PCMCIA | ✅ Linux: Working<br>✅ Windows: Working | A 16-bit PCMCIA card reader was tested successfully, including data transfer, on both operating systems. |
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
| Intel graphics under Windows | ❌ Error 43 | Intel 31.0.101.2140 on SID-fix; NEC 2019 driver causes VIDEO_TDR_FAILURE. [Investigation and test history](windows-graphics.md). |
| FSP-GOP alternative (TestMB) | ❌ Code 43 persists | Build successful; tester reports unchanged Windows failure. [Details](windows-graphics.md#fsp-gop-experiment--2026-11-27). |
| Native libgfxinit (TestMB) | Partial success | eDP and VGA work; Windows Code 43 persists. |
| Native libgfxinit (complete notebook) | ❌ Linux regression | Repeated i915 flip_done and commit wait timeouts on pipe A/eDP-1. |
| HAP/ME experiment | No effect reported | OEM NEC BIOS with HAP clear works; complete A/B matrix and runtime ME state not independently established. [Details](windows-graphics.md#native-libgfxinit-and-hap-follow-up--2026-11-28). |
| External HDMI | ⚠️ Retest needed | No image with coreboot or with the original NEC BIOS on the test board. Linux detects the monitor and reads its EDID under coreboot. The HDMI port is on a daughterboard, and a poor connection is possible. Check the connector and retest with both firmware versions using the same monitor and cable. |
| 64 MiB CardBus prefetchable window (earlier build) | ⚠️ Historical Linux allocation warning | Also observed with original NEC firmware. No separate final-ROM measurement was supplied. |

See the [development problem table](../README.md#development-problems-and-resolutions) for the failed graphics approaches and the changes that restored functionality.

## HDMI GPIO test, 2026-11-24

These notes describe earlier development-build tests; no new final-ROM HDMI pin measurement was supplied. The OEM GPIO snapshot reports GPP_E12 and GPP_E15 as output-high pads (DW0 `0x84000201`), whereas the running coreboot dump showed `0x44000300` for both. A test build configured both with `PAD_CFG_GPO(..., 1, PLTRST)`. The first review-checkout image displayed the TianoCore logo and hung on startup; it was not a valid HDMI test. A subsequent test using the original working development checkout booted, but HDMI still produced no image. The two pad changes did not resolve the issue; their electrical purpose remains unknown. Reverting these experimental changes is recommended. A later test with the original NEC BIOS on the test board also produced no HDMI image. Check the daughterboard connection, then retest both firmware versions with the same monitor and cable.

## NVMe and Intel ME, 2026-11-22

The earlier trimmed ME configuration did not enumerate the NVMe device. Full test-board ME with HAP clear restored enumeration. The latest test with full ME and HAP set also reached the installed Linux system on the NVMe. The earlier reboot loop has no confirmed root cause.

The tested variant uses `descriptor_testmb_hap.bin` derived with `me_cleaner -s` from a full ROM with the original test-board ME. The tested build uses that descriptor and the unchanged `me_testmb.bin` with build-time ME cleaning disabled. The original 16 MiB layout is retained, including ME at `0x3000–0x7fffff` and BIOS at `0x800000–0xffffff`. After building, compare the ME region with the input blob and check HAP using `ifdtool -p sklkbl -d`. The HAP setting is confirmed in the descriptor. The ME runtime state still needs independent validation.

## Internal WLAN RF-kill, 2026-11-23

Before the fix, Linux loaded the AX200 firmware and reported a hardware radio block. The port-based EC read at offset `0x3a` returned `0x00`, and the PCH LPC Generic Memory Range register at PCI offset `0x98` read `0x00000000`. Applying the OEM GPD9 pad configuration alone did not clear the block.

The OEM DSDT writes the EC field `DCWL` at MMIO address `0xfe80013a`, bit 5. The tested board code enables the 64 KiB LPC window at `0xfe800000` in bootblock and sets that bit in the EC ACPI `_INI` method while preserving the other bits. The method skips the write if the read returns `0xff`. It uses the OEM enable path without the `WLAC` and `ELNK` policy branches or the OEM `_Q3B` event handler.

The tester confirmed that internal WLAN works with the retrofitted AX200 after flashing the modified BIOS region. WLAN also works after resume. The individual effects of the two firmware changes and the electrical routing of `W_DISABLE1#` were not tested separately.

## Imaging unit and O2Micro tests, 2026-11-24

The tested notebook has no exposed camera; the display bezel covers the possible camera position. Whether a connector is fitted remains unverified. The Intel Imaging Unit (`8086:1919`, `00:05.0`) appears as an unknown multimedia controller under Windows and is absent from the OEM PCI inventory. The next build should disable the unused imaging unit; the Skylake chipset tree already defaults `imgu` to off and the FSP-S code derives `SaImguEnable` from that device state. Verify the effective state and PCI enumeration before claiming a fix.

An earlier subsystem-ID-only experiment for CardBus and SD did not resolve Windows Code 31. The complete CardBus register sequence reproduced from the OEM firmware resolved Code 31 on the later test build. The early development build used a fixed PCI bus `08` workaround. That workaround is absent from the final upstream port, which was hardware-tested with dynamic bus assignment.

## Historical detailed-test gaps from development builds

- The Secure Boot build uses `SMMSTORE=y` and `EDK2_SECURE_BOOT_SUPPORT=y`, with enforcement off by default in the reference configuration. The ROM has a 512 KiB `SMMSTORE` region at `0x810000`, and its descriptor reports HAP set. The Ubuntu test boot confirmed enforcement.
- Test actual FireWire I/O separately.
- Test Bluetooth pairing separately.
- Validate TPM command execution and measured boot separately. The detection test used `systemd.tpm2_wait=0`.
- Record the negotiated SuperSpeed rate for each port.
- The reference configuration now disables the persistent SPI flash console (`CONFIG_CONSOLE_SPI_FLASH`). Keep CBMEM logging, SPI flash support, and SMMSTORE enabled. The SPI-console-off build still needs hardware validation.
