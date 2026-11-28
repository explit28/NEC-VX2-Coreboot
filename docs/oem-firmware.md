# External firmware and payload inputs

This repository does not distribute proprietary firmware, a complete ROM, or an EDK2 payload binary.

For the NEC VersaPro VX-2 reference configuration, obtain the complete-notebook firmware inputs from a lawful local dump of the target notebook and place them under `3rdparty/blobs/mainboard/nec/vx-2/`:

- `descriptor_hap.bin` — target SPI descriptor with the tested HAP setting.
- `me.bin` — complete original ME region; do not use a cleaned or reduced image.
- `gbe.bin` — target GbE region.
- `IntelGopDriver.efi` — original Intel OEM GOP.
- `IntelGopVbt.bin` — matching OEM VBT.

The build also uses the Kabylake FSP package expected by coreboot's configuration and an EDK2 payload based on MrChromebox revision `44b9aa144a5ff5ceb843245388fb00c87a06895f` with edk2-platforms revision `4db2845d14425599238be86b8a73d288f7c0b4a6`. The tested build enabled the NEC VX-2 panel DXE and diagnostic options from that pinned EDK2 source. Those binaries and source trees are not vendored here.

Do not use firmware blobs from the test motherboard, a different NEC model, or a different machine. Never publish OEM dumps, Intel ME/GbE/descriptor images, GOP/VBT binaries, payload binaries, or complete ROMs in this repository.
