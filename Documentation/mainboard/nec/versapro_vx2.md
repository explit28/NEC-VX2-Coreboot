# NEC VersaPro VX-2

The tested notebook uses the Compal LA-C423P mainboard, an Intel Core
i5-8250U processor, Intel UHD Graphics 620 and two DDR4 SO-DIMM slots.
The SPI flash is a 16 MiB Winbond W25Q128BV in a SOIC-8 package.

The upstream-clean commit series was tested on the target hardware using
the final 16 MiB ROM with SHA-256
`5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`.
The tester reports the hardware validation complete. The SD reader was
tested with normal dynamic PCI bus assignment; no fixed bus number is
required. CardBus window programming, 32-bit CardBus and 16-bit PCMCIA
support were tested with the generic bridge path. The OEM GOP/VBT and
EDK2 payload were also tested. Other previously supported hardware was
included in the completed test; per-port measurements and device models
were not provided for this documentation update.

Firmware images require the board's flash descriptor, Intel ME and GbE
regions. These board-specific binary inputs are not included with
coreboot; obtain them from the target system's own firmware. No ROM or
proprietary firmware blobs are distributed here.

## Flashing

Preserve and verify a backup of the original 16 MiB SPI image before
flashing. An external SPI programmer was used successfully on the test
system. Keep a verified recovery image available.

## Earlier development-image test results

The internal display and graphical UEFI setup, Linux and Windows boot,
NVMe, SATA, external VGA, USB 2.0, USB SuperSpeed, keyboard input in
firmware, Ethernet, audio, Bluetooth and suspend/resume were tested.
An Intel AX200 WLAN adapter was tested, including resume. Secure Boot
settings persist across restarts. Linux detects TPM 2.0. Battery
operation and charging were also confirmed.

SD, 32-bit CardBus and 16-bit PCMCIA data transfers were also tested under
Linux and Windows on an earlier development image. The final upstream-ROM
test above confirmed the supported reader and card types with the upstream
PCI and generic CardBus code; the development-only fixed-bus workaround is
not part of this port.

## Known limitations

- The test system produced no HDMI image with either coreboot or the NEC
  firmware. Linux detects the monitor and reads its EDID with coreboot.
- Linux i915 hardware acceleration works. Under Windows, Intel graphics
  driver 31.0.101.2140 reports Code 43; the NEC 2019 graphics driver can
  trigger `VIDEO_TDR_FAILURE`.
- Windows Intel UHD 620 acceleration remains unavailable with the Intel
  driver because of the known Code 43 issue. The completed hardware test
  did not resolve that limitation.
- The generic CardBus change also updates the Roda RK9 TI PCI7xx1 driver
  to use the shared resource setter. Roda RK9 was not separately built
  or hardware-tested as part of this work.
