# Reference build provenance

This records the owner-reported hardware-tested reference ROM and the saved configuration that accompanied it. The binary is not included in this repository.

- Coreboot version string: `26.09`.
- Base revision: `096fd34d9ca95a267802c25ce697498d41243378`.
- CardBus revision used for the published source: Gerrit #97774, patchset 2, `0ff84145fa0dcf5a6e99fecbc652e9d2b90474a2`.
- Board revision used for the published source: Gerrit #97759, patchset 3, `c02f5c35dbc5a1d083bae7943be3b649d3877992`.
- The earlier hardware-tested CardBus commit has the same Git tree as the current Gerrit CardBus commit. The earlier tested NEC commit differs from the current NEC patchset only in `Documentation/mainboard/nec/versapro_vx2.md`; the source-code tree is unchanged.
- ROM size: 16,777,216 bytes (16 MiB).
- ROM SHA-256: `5e66645f771b2eea21c8275990fc2287b6fc46f9ecb83a0a45d9564a5e0aed13`.
- SPI layout: IFD `0x000000–0x000fff`, GbE `0x001000–0x002fff`, ME `0x003000–0x7fffff`, BIOS `0x800000–0xffffff`.
- Configuration: 128 MiB DVMT preallocation, Intel OEM GOP and VBT, libgfxinit/FSP-GOP/legacy VGA ROM disabled, coreboot IGD/OpRegion path, EDK2 LVGL and UEFI Shell. Secure Boot support is present and starts disabled. TPM 2.0 support remains enabled.
- FSP reported from the image: Kaby Lake FSP `3.7.6.0`.
- EDK2: MrChromebox revision `44b9aa144a5ff5ceb843245388fb00c87a06895f`; edk2-platforms revision `4db2845d14425599238be86b8a73d288f7c0b4a6`.
- The saved test configuration is published as `config/tested-upstream.config`. The notebook serial-number setting was removed and the build-local payload path was normalized; other settings were retained.

The test report states that the original notebook descriptor, complete ME and GbE images, OEM GOP and VBT were used, and that their regions/assets were structurally compared during ROM validation. Do not substitute TestMB blobs or a cleaned/reduced ME image.

## Reproducing the inputs

Provide these complete-notebook files locally under `3rdparty/blobs/mainboard/nec/vx-2/`:

- `descriptor_hap.bin`
- `me.bin`
- `gbe.bin`
- `IntelGopDriver.efi`
- `IntelGopVbt.bin`

Also provide the matching Kabylake FSP files from coreboot's external FSP package and build the EDK2 payload from the pinned revisions above. Place the resulting payload at the path selected in the sanitized reference configuration (`payloads/UEFIPAYLOAD.fd`). These binary inputs are not included here because they contain proprietary firmware.

Verify the descriptor, FMAP/CBFS regions, payload and final SHA-256 locally. Keep a verified original ROM and recovery method. Hardware testing is not a substitute for checking that the image is for the intended system.
