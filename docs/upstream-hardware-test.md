# Final upstream hardware test — NEC VersaPro VX-2

Date reported: 2026-11-28. The user confirms completion of the full hardware test on the final upstream ROM.

## Tested image and source provenance

- ROM size: 16 MiB. SHA-256: `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`.
- Tested source series: Coreboot base `096fd34d9ca95a267802c25ce697498d41243378`, CardBus `94fe8fa9504b97b0bebedbe0d20827d86e886526`, NEC `3e34346260337fd245533e47a40da18ffd45cf12`. The NEC changes between that tested source and the final submitted tree were documentation-only.
- EDK2 MrChromebox `44b9aa144a5ff5ceb843245388fb00c87a06895f`; `edk2-platforms` `4db2845d14425599238be86b8a73d288f7c0b4a6`; Kaby Lake FSP `3.7.6.0`.
- OEM GOP and VBT were used. The ROM, descriptor, ME, GbE and OEM GOP/VBT binaries are not published in this repository.
- Final Gerrit revisions: [#97774](https://review.coreboot.org/c/coreboot/+/97774), CardBus `9acf4b12a5e9ee5f1b96c64e27deeb23a24f3b70`, Change-ID `I9a9247b7294720b599f0af9e9f421c1186163093`; [#97759](https://review.coreboot.org/c/coreboot/+/97759), NEC `9430870094bdd7754ea89c05441d7b8c54db2621`, Change-ID `Ic47966b6d08122709f3d73ad5c3634956e62dcc8`, depending on #97774. The message-only corrections preserved the final trees; the submitted NEC tree includes only documentation changes after the tested source.
- The preserved OEM-GOP reference uses the Intel OEM GOP/VBT with EDK2; its saved configuration has `CONFIG_NO_GFX_INIT=y` and `CONFIG_EDK2_GOP_DRIVER=y`. Keep it as the baseline for future work; no new Code 43 analysis was performed.

## Reported test scope

| Area | Reported status | Detail supplied |
|---|---|---|
| SD reader | Tested with dynamic PCI bus allocation | No exact bus number or observed BDF was supplied. Upstream does not force bus `0x08`. |
| Generic CardBus windows | Tested | No final register dump or window sizes supplied. |
| 32-bit CardBus | Tested | No final card model or transfer-rate data supplied. |
| 16-bit PCMCIA | Tested | No final card model or transfer-rate data supplied. |
| OEM GOP/VBT and EDK2 | Tested | No separate per-resolution transcript supplied. |
| Previously supported notebook hardware | Included in completed full test | Individual port and device outcomes were not separately provided. |
| Windows UHD 620 acceleration | Known issue remains | Windows continues to report Code 43. No new diagnosis is part of this report. |
| Roda RK9 | Not tested | No separate build or hardware test was performed. |

The test report does not provide per-port measurements, exact SD bus numbering, card models, or new Windows diagnostic output. This document does not infer those details. Earlier development-build results remain in [the historical test matrix](test-matrix.md).

## Generic CardBus and development separation

The CardBus commit programs the base and inclusive limit for each I/O and memory window, uses Coreboot resource assignment, validates alignment and 32-bit limits, closes unassigned or invalid windows, and keeps prefetch on Memory Window 0. The same generic setter is used by the default CardBus path and the Roda TI PCI7xx1 driver, but Roda itself was not tested.

The SD reader uses normal dynamic Coreboot PCI bus assignment; the old fixed-bus-08 experiment is historical and was removed from the upstream source. The experimental IGD work remains preserved separately from the cleaned upstream port. The code changes are published through the two Gerrit changes linked above. See [development notes](development-notes.md) and [Windows graphics history](windows-graphics.md).
