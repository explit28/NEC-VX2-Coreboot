# Development notes — NEC VersaPro VX-2

## Graphics experiments (2026-11-22)

**libgfxinit:** probe reported ports EDP, HDMI1, DP2, HDMI2 valid; eDP AUX I2C and HDMI DDC did not return EDID; no primary/secondary display selected; black display.

**Legacy VBIOS:** Intel `8086:5917` ROM, 65,536 bytes, reported successful invocation. Coreboot's PCI option ROM search first tried mapped `8086:0406` then used `8086:5917`. VBE `0x118` produced reported 1024×768×32 framebuffer at `0xf0000000`. A subsequent `0x11b` 1280×1024 trial also produced illuminated panel but no visible pixels. This only proves ROM execution and reported mode-setting, not successful eDP scan-out.

**OEM IntelGopDriver + IntelGopVbt:** `NO_GFX_INIT=y`, `INTEL_GMA_ADD_VBT=y`, `EDK2_GOP_DRIVER=y` with MrChromebox EDK2. Build log: `Using GOP driver ExPLIT/NEC-GOP/IntelGopDriver.efi`. The EDK2 workspace had GOP (69,568 bytes) and VBT (4,608 bytes); CBFS listed `vbt.bin` with 4,608-byte uncompressed size and `fallback/payload`. A 16 MiB coreboot image was built. **On real hardware the coreboot logo appeared and UEFI Shell started.** SSD absent at time of observation.

## Windows graphics compatibility (2026-11-27)

Linux Intel graphics works with the hardware-verified SID correction. Windows reports Code 43 with Intel 31.0.101.2140 and VIDEO_TDR_FAILURE with the NEC 2019 driver. The [dedicated Windows graphics investigation](windows-graphics.md) records the firmware changes, diagnostic readings, driver A/B tests and open issues.

## Native libgfxinit and HAP follow-up — 2026-11-28

- Native libgfxinit TestMB build: coreboot `0c42e2ee1fe1179bbb973765493842141b228686`, libgfxinit `7ac11bc0747628aec87d074643b54038aa08e1d4`, BIOS version 26.09. Both eDP and VGA output worked on the test board, but Windows still reported Code 43. The earlier AUX/EDID failure no longer prevented display output; the exact responsible upstream change is not established. OEM GOP, FSP-GOP and legacy VGA ROM execution were disabled. EDK2 used the native framebuffer; the generic GOP path lacks EDID protocols used by the panel DXE.
- Full VX-2 notebook: native libgfxinit produced a Linux regression. The photographed i915 log repeatedly reports `flip_done timed out` and `commit wait timed out` for pipe A, plane 1A and eDP-1 after reaching graphical.target. The root cause is not established. Successful firmware display output is not evidence of a successful Linux graphics handoff.
- HAP/ME experiment: the original TestMB firmware is `VX-2_testmb.rom`. The intended controlled comparison preserves the OEM BIOS, full ME and GbE and changes only the descriptor HAP bit. The tester reports that HAP has no effect on Code 43 and that graphics works normally on the original NEC BIOS with HAP disabled (bit clear). The precise outcomes for OEM HAP set and coreboot HAP clear were not separately stated, and no runtime ME status or final binary comparison was supplied. Therefore record the tester's negative conclusion without claiming a fully verified four-way A/B matrix or excluding every ME/coreboot interaction.
- Decision: return to the stable OEM Intel GOP + original VBT in EDK2, with the existing consolidated upstream review [#97759](https://review.coreboot.org/c/coreboot/+/97759) as the source baseline. Do not promote the experimental FSP-GOP or libgfxinit configuration. Keep the experimental artifacts for diagnosis. The rollback is requested, not yet verified; no new Gerrit change is needed.
- Windows Code 43 remains open. Prioritize OEM-versus-coreboot comparisons of ACPI/OpRegion, system-agent policy, PCI resources, interrupts and GPU state rather than repeating the three graphics initialization experiments.

## Secure Boot follow-up

The later 16 MiB build combined the full test-board ME and HAP descriptor with SMMSTORE and MrChromebox EDK II Secure Boot support. Its ROM layout reported a 512 KiB SMMSTORE region at `0x810000`. Ubuntu booted from NVMe through `shimx64.efi` with Secure Boot enabled and kernel lockdown active. The earlier graphics test was performed without an SSD. See the [test matrix](test-matrix.md) for current results.

## Caveats

- `CONFIG_RUN_FSP_GOP` is **not** the mechanism used. That config is for an FSP-integrated graphics path, distinct from EDK2's external GOP driver.
- The OEM GOP may depend on board hardware initialization; retain the full successful `.config` and the complete mainboard directory.
- Keep full original firmware images private. Do not copy binary blobs into a public repo by default.
- Prefer including exact commit IDs and checksums in a local/private release manifest; extract from the actual checkout rather than inferring them.
