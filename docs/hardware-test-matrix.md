# Hardware test status

The owner reports that the final 16 MiB upstream-review ROM was fully tested on NEC VersaPro VX-2 hardware (ROM SHA-256 is in [build provenance](build-provenance.md)). The report specifically confirms continued use of the dynamically enumerated SD reader, 32-bit CardBus, 16-bit PCMCIA, and the OEM GOP/EDK2 path. Detailed per-device logs, register values, card models and measurements were not provided for that final run. Earlier results below are retained as project observations, not represented as new measurements from the final run.

| Area | Reported status | Notes |
|---|---|---|
| Boot and preboot graphics | Tested | OEM Intel GOP/VBT with EDK2; graphical LVGL menu and UEFI Shell are included. VGA output was previously confirmed. |
| Linux | Previously tested | Linux boot and Intel graphics acceleration were previously reported working with the OEM GOP configuration. No final-run benchmark or log was supplied. |
| Windows | Boots; graphics limitation remains | Intel UHD 620 reports Code 43. No new graphics diagnosis was done for the final retest. |
| CardBus / PCMCIA | Tested | Generic bridge memory/I/O base and inclusive limit programming; 32-bit CardBus and 16-bit PCMCIA support reported working. Exact register readbacks and card models are unavailable. |
| SD reader | Tested | O2 Micro `1217:8621`; dynamic PCI bus assignment. The actual bus number and physical root-port function observed during the final run were not supplied. |
| USB, SATA, NVMe, LAN/WLAN, audio, TPM 2.0, keyboard/touchpad, battery and suspend/resume | Previously supported; included in the reported complete retest | No port-by-port results or component-specific measurements were supplied for the final run. WLAN tests used a retrofitted Intel AX200 in earlier development. |
| HDMI | Unresolved | Earlier reports recorded no image with coreboot and the original NEC BIOS; HDMI is on a daughterboard. Final retest details were not supplied. |
| Roda RK9 | Not tested separately | The generic CardBus change also touches its TI PCI7xx1 driver. No separate Roda RK9 build or hardware test was performed. |

## CardBus register reference

The standard CardBus configuration-space windows use base/limit pairs:

| Window | Base | Limit |
|---|---:|---:|
| Memory 0 | `0x1c` | `0x20` |
| Memory 1 | `0x24` | `0x28` |
| I/O 0 | `0x2c` | `0x30` |
| I/O 1 | `0x34` | `0x38` |

The generic fix retains the existing prefetchable assignment for Memory Window 0. Functional card support was reported, but a final PCI configuration-space dump was not supplied.
