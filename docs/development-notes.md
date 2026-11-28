## Final upstream hardware test — 2026-11-28

The user reports completing the full hardware test of the final upstream ROM (SHA-256 `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`). The tested series is Coreboot base `096fd34d9ca95a267802c25ce697498d41243378`, generic CardBus `94fe8fa9504b97b0bebedbe0d20827d86e886526`, and NEC board port `7ff147979f5e2e7b4d9245e8553f37a596d260ae`.

The reported test covered OEM GOP/VBT with EDK2, the SD reader using dynamic PCI bus assignment, generic CardBus window programming, 32-bit CardBus, 16-bit PCMCIA, and previously supported notebook hardware. The exact SD bus number, per-port observations, card models, and register measurements were not supplied. Windows Intel UHD 620 Code 43 remains unresolved. No new graphics investigation was performed in this update.

Upstream and experiment remain separate: `nec-vx2-upstream-review` contains the clean review series; `nec-vx2-development-backup` retains the earlier experimental IGD work. The GitHub source tree preserves that development history and adds source-only upstream patch files. No full ROM or proprietary firmware blobs are published. Roda RK9 was not separately built or tested.

# Development notes — NEC VersaPro VX-2

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
