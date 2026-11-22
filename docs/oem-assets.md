# OEM assets — private acquisition, no redistribution

The build used locally extracted OEM components. The source archive does **not** include these binaries.

### Expected local locations (relative to coreboot root)

```text
ExPLIT/NEC-GOP/IntelGopDriver.efi                 # 69,568 B
3rdparty/blobs/mainboard/nec/vx-2/vbt.bin         # 4,608 B
```

The successful `.config` refers to these paths. The GOP module was extracted from the **PE32 image section body** of `IntelGopDriver` (GUID `FF0C8745-3270-4439-B74F-3E45F8C77064`). The VBT came from the **Raw section body** of `IntelGopVbt` (GUID `878AC2CC-5343-46F2-B563-51F89DAF56BA`).

Check the VBT before building:

```sh
sha256sum 3rdparty/blobs/mainboard/nec/vx-2/vbt.bin
# expected for tested VBT:
# 407ceb6c1a3eafafc9aa441207bc23d69728465ebc046997d300b7052da2ad33
```

For the successful test, the VBT originally located at that board blob path was replaced with the 4,608-byte VBT extracted from the OEM GOP package; the previous VBT was backed up locally.

Avoid committing OEM flash dumps, Intel ME partitions, EC images, private keys, serial numbers, MAC addresses, UUIDs or NVRAM variables.
