# Development notes — NEC VersaPro VX-2

## Graphics experiments (2026-11-22)

**libgfxinit:** probe reported ports EDP, HDMI1, DP2, HDMI2 valid; eDP AUX I2C and HDMI DDC did not return EDID; no primary/secondary display selected; black display.

**Legacy VBIOS:** Intel `8086:5917` ROM, 65,536 bytes, reported successful invocation. Coreboot's PCI option ROM search first tried mapped `8086:0406` then used `8086:5917`. VBE `0x118` produced reported 1024×768×32 framebuffer at `0xf0000000`. A subsequent `0x11b` 1280×1024 trial also produced illuminated panel but no visible pixels. This only proves ROM execution and reported mode-setting, not successful eDP scan-out.

**OEM IntelGopDriver + IntelGopVbt:** `NO_GFX_INIT=y`, `INTEL_GMA_ADD_VBT=y`, `EDK2_GOP_DRIVER=y` with MrChromebox EDK2. Build log: `Using GOP driver ExPLIT/NEC-GOP/IntelGopDriver.efi`. The EDK2 workspace had GOP (69,568 bytes) and VBT (4,608 bytes); CBFS listed `vbt.bin` with 4,608-byte uncompressed size and `fallback/payload`. A 16 MiB coreboot image was built. **On real hardware the coreboot logo appeared and UEFI Shell started.** SSD absent at time of observation.

## Caveats

- The successful test does not independently verify disk detection, boot order, or booting an OS.
- `CONFIG_RUN_FSP_GOP` is **not** the mechanism used. That config is for an FSP-integrated graphics path, distinct from EDK2's external GOP driver.
- The OEM GOP may depend on board hardware initialization; retain the full successful `.config` and the complete mainboard directory.
- Keep full original firmware images private. Do not copy binary blobs into a public repo by default.
- Prefer including exact commit IDs and checksums in a local/private release manifest; extract from the actual checkout rather than inferring them.
