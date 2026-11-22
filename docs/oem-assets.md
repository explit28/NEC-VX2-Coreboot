# OEM assets — private acquisition, no redistribution

The build used locally extracted OEM components. The source archive does **not** include these binaries.

## Required local inputs for the published configuration

All paths are relative to the coreboot source root. The files under `3rdparty/blobs/mainboard/nec/vx-2/` are board-specific local inputs. Initializing coreboot submodules does not supply those NEC files.

| File | Configuration | Source and purpose |
| --- | --- | --- |
| `3rdparty/blobs/mainboard/nec/vx-2/descriptor_testmb.bin` | `CONFIG_HAVE_IFD_BIN=y` | Intel Flash Descriptor region extracted from the working 16 MiB `nec-vx2-oem-gop-test12.rom` on the test board. Defines the flash region layout. |
| `3rdparty/blobs/mainboard/nec/vx-2/me_testmb.bin` | `CONFIG_HAVE_ME_BIN=y` | Intel Management Engine region extracted from the same working test-board ROM. The captured build configuration has `CONFIG_USE_ME_CLEANER` disabled. |
| `3rdparty/blobs/mainboard/nec/vx-2/gbe.bin` | `CONFIG_HAVE_GBE_BIN=y` | Intel Gigabit Ethernet region from the original NEC SPI dump. Preserve the target machine's own region, which can contain its MAC address. |
| `ExPLIT/pci8086,5917.rom` | `CONFIG_VGA_BIOS=y` | Intel legacy VGA option ROM used as a build input by the captured configuration (PCI ID `8086:5917`). |
| `ExPLIT/NEC-GOP/IntelGopDriver.efi` | `CONFIG_EDK2_GOP_DRIVER=y` | NEC OEM Intel GOP DXE driver for EDK2, 69,568 bytes. |
| `3rdparty/blobs/mainboard/nec/vx-2/vbt.bin` | `CONFIG_INTEL_GMA_ADD_VBT=y` | Matching NEC OEM Video BIOS Table, 4,608 bytes. |

`CONFIG_VGA_BIOS=y` includes the legacy option ROM in the build, while `CONFIG_VGA_ROM_RUN` is disabled for coreboot. The visible splash was observed with NEC GOP in EDK2. A build with the legacy VGA ROM omitted has not been tested.

Keep SPI dumps private. For a different machine, retain its own firmware backups and verify the descriptor layout and ME/GbE inputs before building. The exact hashes of those three inputs were not captured in the source snapshot.

## Binary dependencies supplied by coreboot submodules

| Dependency | Source |
| --- | --- |
| Intel Kaby Lake FSP | `3rdparty/fsp/KabylakeFspBinPkg/Fsp.fd` from the Intel FSP submodule. The build derives `Fsp_M.fd` and `Fsp_S.fd` from this input. |
| Intel CPU microcode | Default Intel microcode from coreboot's `3rdparty/intel-microcode` submodule. The configuration uses `CONFIG_CPU_MICROCODE_CBFS_DEFAULT_BINS=y`. |

Initialize the submodules in the coreboot checkout with `git submodule update --init --recursive`. The build also generates the EDK2 payload at `build/UEFIPAYLOAD.fd`.

## Extracting the OEM graphics files

The GOP module was extracted from the **PE32 image section body** of `IntelGopDriver` (GUID `FF0C8745-3270-4439-B74F-3E45F8C77064`). The VBT came from the **Raw section body** of `IntelGopVbt` (GUID `878AC2CC-5343-46F2-B563-51F89DAF56BA`).

Check the VBT before building:

```sh
sha256sum 3rdparty/blobs/mainboard/nec/vx-2/vbt.bin
# expected for tested VBT:
# 407ceb6c1a3eafafc9aa441207bc23d69728465ebc046997d300b7052da2ad33
```

For the successful test, the VBT originally located at that board blob path was replaced with the 4,608-byte VBT extracted from the OEM GOP package; the previous VBT was backed up locally.

Avoid committing OEM flash dumps, Intel ME partitions, EC images, private keys, serial numbers, MAC addresses, UUIDs or NVRAM variables.
