# Windows graphics compatibility — Intel UHD 620

Status: 2026-11-28. Open firmware compatibility issue; Linux graphics works, Windows native Intel graphics reports Device Manager Code 43 with the 2025 driver. The older NEC driver can cause VIDEO_TDR_FAILURE (0x116). Windows boot and GOP output are not evidence of working Windows acceleration.

## Hardware and test matrix

IGD PCI 00:02.0, 8086:5917; OEM subsystem 17AA:60CF. Results below are hardware tester observations; not all earlier builds were tested with identical memory or Windows configuration.

| Firmware | Subsystem | Driver | Result |
|---|---|---|---|
| Original NEC BIOS | 17AA:60CF | 25.20.100.6577 (2019-02-07) | Windows graphics works |
| Original NEC BIOS | 17AA:60CF | 31.0.101.2140 (2025-11-05) | Windows graphics works |
| Early coreboot / DVMT128 | 8086:2212 | Intel 2025 | Code 43 persists |
| Combined V2 | 17AA:2212 | Intel 2025 | Code 43 |
| SID fix | 17AA:60CF | NEC 2019 | Both internal and external display cycle black/bright; VIDEO_TDR_FAILURE |
| SID fix | 17AA:60CF | Intel 2025 | Code 43 after manual switch in Safe Mode and normal reboot |
| SID fix | 17AA:60CF | Linux i915 | Working (tester confirmed); OpenGL 4.6 hardware acceleration confirmed on OEM-GOP firmware |
| FSP-GOP TestMB | 17AA:60CF (configured) | Windows Intel driver (version not rechecked) | Code 43; same reported behavior as OEM GOP |

Safe Mode works. The external monitor exhibited the same Windows failure; this is not evidence that HDMI specifically has been repaired. Earlier HDMI-specific hardware issue remains separate.

## Development history and retained changes

- libgfxinit failed to obtain panel EDID over AUX; legacy VBIOS produced no visible pixels. OEM GOP/VBT through MrChromebox EDK2 restores graphical preboot output. Keep this path as the known working preboot reference; the subsequent native libgfxinit experiment restored preboot output but introduced a Linux regression on the complete notebook.
- OEM GGC 0x01C1 corresponds to 32 MiB preallocation and 8 MiB GGTT; 128 MiB was selected deliberately for testing, not claimed to be the OEM physical allocation. FSP-M IgdDvmt50PreAlloc=0x04 produces GGC 0x04C1. Increasing DVMT did not fix Code 43.
- Early DSM/PCON and subsystem experiments did not resolve Code 43; earlier DWORD PreMem subsystem write caused Windows regression, then reverted.
- OEM compatibility V1/V2 added board-scoped graphics AML and CBMEM-backed graphics state, partial DSM functions, guarded SWSCI-related methods. V2 full build tested: Code 43 unchanged. SWSCI is deliberately not activated.
- Combined V1 retained DVMT128, experimental integer DSM return and brightness valid-bit changes. Combined V2 superseded its brightness implementation with Post-GOP DXE, added ACPI NVS reservation, live EDID handoff, panel brightness curve, EC lid state and additional guarded DSM support. See [build instructions](combined-v2-build.md). Code 43 persisted.
- Combined V2 normal PCI subsystem programming corrected SVID only. FSP-M analysis identified the early SID latch; guarded board-local 16-bit SID write before FSP-M corrected it without writing SVID prematurely. See [SID analysis](igd-ssid.md).
- Retain the SID-fix and related graphics changes as experimental, not as a Windows graphics fix. This historical record does not imply any firmware, ME or IFD binary publication.

## Post-GOP EFI diagnostic results

| Field | Diagnostic 00 (DVMT128) | Diagnostic 01 (Combined V2) | Diagnostic 02 (SID fix) |
|---|---|---|---|
| PCI subsystem DWORD | 22128086 | 221217AA | 60CF17AA |
| IGD command | 0007 | 0007 | 0007 |
| GGC / MSAC | 04C1 / 01 | 04C1 / 01 | 04C1 / 01 |
| BDSM / BGSM | 78000001 / 77800001 | same | same |
| TOLUD | 80000001 | same | same |
| ASLS | 76A3E000 | 76A3D000 | 76A3D000 |
| OpRegion memory type | Reserved | ACPI NVS | ACPI NVS |
| OpRegion mailbox mask / PCON | 1D / 0 | 1D / 0 | 1D / 0 |
| PHED / EDID | 0 / absent | 1 / valid 128 bytes | 1 / valid 128 bytes |
| BCLP | FF | 800000FF | 800000FF |

Diagnostic 02 reports PCI bytes AA 17 CF 60 at 0x2C; SID fix verified on hardware. ACPI NVS covers exactly 2 pages at ASLS. GOP 1920x1080, framebuffer D0000000, EDID LG Display LP156WF6-SPK6. BCLM populated, CLID=1, PFIT=80000006, CBLV=80000064. Reported sampled power well registers F000000F/50000005/40000000/50000005, FUSE_STATUS=8E000000, CDCLK_CTL=0C000544, STOLEN_RESERVED=0 and initial eight GGTT PTEs mapping 78000000 through 78007000 match the earlier reported readings. This does not prove full render/PPGTT operation or identical unmeasured hardware state.

Physical DVMT reservation is confirmed: BDSM masked=78000000, TOLUD masked=80000000 (128 MiB); BGSM masked=77800000 (8 MiB GGTT). Windows previously reported 0 graphics memory; this is not proof of absent physical reservation. Current Windows memory reporting after the driver switch has not been measured.

## Windows driver selection and crash analysis

SetupAPI analysis in the diagnostic session reported that the exact SUBSYS_60CF17AA match selects oem21.inf (iigd_dch1.inf, rank 00CF0001) over generic oem45.inf (iigd_dch.inf, rank 00CF2001). The NEC extension oem3.inf (iigd_ext1.inf) also matches the OEM ID. INF excerpts directly confirm those hardware matches and versions. The 2025 driver was manually selected in Safe Mode (version and signature photographed), then normal boot returned Code 43. The effective extension and post-reboot binding should be verified separately.

Both base INF Kaby Lake sections specify FeatureTestControl=0x8200, PSR2Disable=0, Psr2DrrsEnable=1, LFP_Brightness_Control=0, DisplayFeatureControl=0x3E3, MSISupported=1, ForcePLLDisableEnableForFreqChange=0 and SkipAudioPowerWellConfigInCS=0. The 2019 section additionally specifies ControlDACDriverLoadOrder=1; the 2025 section adds KmFtrControl=3. NEC extension specifies FeatureTestControl=0x8220, DisableEdramForDisplayRT=1 (comment targets GT3e flicker; this GPU is GT2), and ShowOptimalBalloon=0. Exact semantics of bit 5 and KmFtrControl are unverified; INF values do not establish effective registry state.

The tester directly observed VIDEO_TDR_FAILURE. Earlier session dump analysis reported 0x116 with Arg3=0 and Arg4=0xD, plus igdkmd64.sys references; no symbol-assisted WinDbg analysis has been completed. Treat detailed dump interpretation as provisional. [Microsoft bugcheck reference](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/bug-check-0x116---video-tdr-failure).

VBS/HVCI/hypervisor disabled test on an earlier build did not resolve Code 43; VT-d itself was not excluded. Restoration of the security settings should be checked.

## FSP-GOP experiment — 2026-11-27

**Unsuccessful Windows graphics fix.** The tester flashed and tested the TestMB image and reports the same Windows Code 43 as with the OEM GOP in EDK2. The build succeeded; this is a runtime failure, not a compilation failure.

- Coreboot base: `0c42e2ee1fe1179bbb973765493842141b228686`; displayed version 26.09.
- ROM: `NEC-VX2-IGD-FSP-GOP-TestMB.rom`, SHA256 `58570a7ab60a84a54345e181df76af770923cc8512267b38003135ee601090be` (verified against supplied ROM).
- RUN_FSP_GOP enabled, NO_GFX_INIT and EDK2 OEM GOP disabled. FSP-S receives the original NEC VBT; EDK2 GraphicsOutputDxe consumes the framebuffer handoff. LVGL retained.
- DVMT128, OEM SID, OpRegion ACPI NVS and experimental DSM retained in configuration; SWSCI inactive. Kconfig expanded to allow FSP-GOP; EDK2 DSC/FDF keep Vx2PanelDxe outside the platform GOP conditional.
- The generic framebuffer GOP does not provide EDID protocols in the inspected source path. Vx2PanelDxe may skip panel metadata; no runtime EDID/OpRegion dump was supplied for this test.
- Test board uses its own descriptor_testmb_hap.bin, gbe_testmb.bin and full me_testmb.bin; ME cleaner disabled. Build report confirms region comparisons and HAP set. Proprietary binaries remain local.
- The supplied build log shows successful AML compilation (0 errors, 0 warnings) and a completed ROM.

Changing the GOP initialization path alone did not resolve Code 43. This weakens the hypothesis that the EDK2 OEM GOP path alone is responsible; it does not exclude shared FSP policy, ACPI, GPU state or differences in EDID handoff. Linux acceleration was confirmed separately on the earlier OEM-GOP firmware (Mesa Intel UHD 620, OpenGL 4.6, Accelerated: yes); it has not been independently retested on this FSP build.

The native libgfxinit follow-up is documented below.

## Native libgfxinit and HAP follow-up — 2026-11-28

- Native libgfxinit TestMB build: coreboot `0c42e2ee1fe1179bbb973765493842141b228686`, libgfxinit `7ac11bc0747628aec87d074643b54038aa08e1d4`, BIOS version 26.09. Both eDP and VGA output worked on the test board, but Windows still reported Code 43. The earlier AUX/EDID failure no longer prevented display output; the exact responsible upstream change is not established. OEM GOP, FSP-GOP and legacy VGA ROM execution were disabled. EDK2 used the native framebuffer; the generic GOP path lacks EDID protocols used by the panel DXE.
- Full VX-2 notebook: native libgfxinit produced a Linux regression. The photographed i915 log repeatedly reports `flip_done timed out` and `commit wait timed out` for pipe A, plane 1A and eDP-1 after reaching graphical.target. The root cause is not established. Successful firmware display output is not evidence of a successful Linux graphics handoff.
- HAP/ME experiment: the original TestMB firmware is `VX-2_testmb.rom`. The intended controlled comparison preserves the OEM BIOS, full ME and GbE and changes only the descriptor HAP bit. The tester reports that HAP has no effect on Code 43 and that graphics works normally on the original NEC BIOS with HAP disabled (bit clear). The precise outcomes for OEM HAP set and coreboot HAP clear were not separately stated, and no runtime ME status or final binary comparison was supplied. Therefore record the tester's negative conclusion without claiming a fully verified four-way A/B matrix or excluding every ME/coreboot interaction.
- Decision: return to the stable OEM Intel GOP + original VBT in EDK2, with the existing consolidated upstream review [#97759](https://review.coreboot.org/c/coreboot/+/97759) as the source baseline. Do not promote the experimental FSP-GOP or libgfxinit configuration. Keep the experimental artifacts for diagnosis. The rollback is requested, not yet verified; no new Gerrit change is needed.
- Windows Code 43 remains open. Prioritize OEM-versus-coreboot comparisons of ACPI/OpRegion, system-agent policy, PCI resources, interrupts and GPU state rather than repeating the three graphics initialization experiments.

## Open investigation

- OEM OpRegion mailbox mask 0x1F at initialization versus current 0x1D; OEM PCON policy versus current zero; GOP DVER remains empty. OEM runtime OpRegion snapshot still missing.
- Complete OEM DSM behavior, especially function 0x0F GetInternalGraphics, and policy dependencies. Do not advertise unsupported functions or blindly copy OEM AML.
- SWSCI GBDA/SBCB and shared GPE _L66 semantics; do not enable mailbox 2 until full safe handler and event routing are validated.
- Compare OEM and coreboot power-management, interrupts, render reset, GGTT/PPGTT, PAVP and DMAR/VT-d. No confirmed causal register yet.
- Obtain symbol-assisted crash analysis and current Windows problem status, driver/extension binding and AdapterRAM. Avoid changing TDR timeout or disabling recovery.
- Compare with related Kaby Lake ThinkPad implementations while retaining the working OEM GOP path. Test both Windows drivers, Linux acceleration and existing peripherals after future changes.

Success requires native Windows Intel driver starts without Code 43 or TDR, acceleration and stable internal/external display tested, and no Linux/peripheral regressions. The SID correction alone is verified, not sufficient.

## Provenance and privacy

Reference local coreboot commits d072b8f075a5935015313c45b74cc1f67fed79b0 (Combined V2), 3a8c0b4108fbc274dca7fc53fecd84b679b0ecfd (SID fix), EDK2 44b9aa144a5ff5ceb843245388fb00c87a06895f, as reported by the build manifest. SID-fix ROM SHA256 201e121bce150176cd943e5b92d47a5f2eb344be58c95e69cff3abd2e8b903d4 (manifest value). Local artifacts remain under NEC-VX2-Testbuild within the existing coreboot checkout. Publish sanitized findings only, not Windows dumps, registry hives, OEM binary assets, device identifiers beyond hardware IDs, or raw private logs.
## Final upstream ROM retest — 2026-11-28

The user reports a complete hardware test of the final upstream ROM (SHA-256 `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`). Windows Intel UHD 620 Code 43 remains a known limitation. The report did not include a new driver-version A/B, diagnostic dump, register snapshot or root-cause finding. No new Code 43 analysis was performed for this retest. The final ROM's OEM GOP/EDK2 preboot path was tested; preboot graphics success is separate from Windows 3D acceleration. See [final test provenance](build-provenance.md#final-upstream-rom-hardware-test--2026-11-28).
