# OEM assets — private acquisition, no redistribution

The build used locally extracted OEM components. The source archive does **not** include these binaries.

## Required local inputs for the published configuration

All paths are relative to the coreboot source root. The files under `3rdparty/blobs/mainboard/nec/vx-2/` are board-specific local inputs. Initializing coreboot submodules does not supply those NEC files.

| File | Configuration | Source and purpose |
| --- | --- | --- |
| `3rdparty/blobs/mainboard/nec/vx-2/descriptor_hap.bin` | `CONFIG_HAVE_IFD_BIN=y` | Intel Flash Descriptor with HAP set and the original flash layout preserved. Prepare it as described below. |
| `3rdparty/blobs/mainboard/nec/vx-2/me.bin` | `CONFIG_HAVE_ME_BIN=y` | Full Intel Management Engine region from the target machine’s original SPI dump. The captured build configuration has `CONFIG_USE_ME_CLEANER` disabled. |
| `3rdparty/blobs/mainboard/nec/vx-2/gbe.bin` | `CONFIG_HAVE_GBE_BIN=y` | Intel Gigabit Ethernet region from the original NEC SPI dump. Preserve the target machine's own region, which can contain its MAC address. |
| `3rdparty/blobs/mainboard/nec/vx-2/pci8086,5917.rom` | `CONFIG_VGA_BIOS=y` | Intel legacy VGA option ROM used as a build input by the captured configuration (PCI ID `8086:5917`). |
| `3rdparty/blobs/mainboard/nec/vx-2/IntelGopDriver.efi` | `CONFIG_EDK2_GOP_DRIVER=y` | NEC OEM Intel GOP DXE driver for EDK2, 69,568 bytes. |
| `3rdparty/blobs/mainboard/nec/vx-2/IntelGopVbt.bin` | `CONFIG_INTEL_GMA_ADD_VBT=y` | Matching NEC OEM Video BIOS Table, 4,608 bytes. |

`CONFIG_VGA_BIOS=y` includes the legacy option ROM in the build, while `CONFIG_VGA_ROM_RUN` is disabled for coreboot. The visible splash was observed with NEC GOP in EDK2. A build with the legacy VGA ROM omitted has not been tested.

Keep SPI dumps private. For a different machine, retain its own firmware backups and verify the descriptor layout and ME/GbE inputs before building. The exact hashes of those three inputs were not captured in the source snapshot.

## Binary dependencies supplied by coreboot submodules

| Dependency | Source |
| --- | --- |
| Intel Kaby Lake FSP | `3rdparty/fsp/KabylakeFspBinPkg/Fsp.fd` from the Intel FSP submodule. The build derives `Fsp_M.fd` and `Fsp_S.fd` from this input. |
| Intel CPU microcode | Default Intel microcode from coreboot's `3rdparty/intel-microcode` submodule. The configuration uses `CONFIG_CPU_MICROCODE_CBFS_DEFAULT_BINS=y`. |

Initialize the submodules in the coreboot checkout with `git submodule update --init --recursive`. The build also generates the EDK2 payload at `build/UEFIPAYLOAD.fd`.

## Extract the OEM inputs from your own NEC firmware

Use a **full 16 MiB SPI dump from the target NEC VersaPro VX-2**, saved as `VX-2-original.rom` in the coreboot checkout. A vendor update capsule or a BIOS-region-only file cannot supply the descriptor, ME and GbE regions. Use a verified backup from the target board and keep an untouched copy. All steps below read the source image only.

### 1. Extract Flash Descriptor, Intel ME and GbE

Use coreboot's `ifdtool`. Set `ifdtool=ifdtool` if the tool is installed on your PATH. The commands check the observed factory layout and stop on a different layout. For a different verified layout, use `ifdtool -p sklkbl -x` in an empty directory and review the extracted regions before choosing a coreboot configuration.

```bash
rom="$PWD/VX-2-original.rom"
ifdtool="$PWD/build/util/ifdtool/ifdtool"
out=$(mktemp -d "$PWD/vx2-oem-blobs.XXXXXX")
printf 'Save all six files in %s\n' "$out"

test "$(stat -c %s "$rom")" -eq 16777216 &&
layout=$("$ifdtool" -p sklkbl -d "$rom") &&
grep -Ei 'Flash Region 0 .*00000000 - 00000fff' <<< "$layout" &&
grep -Ei 'Flash Region 1 .*00800000 - 00ffffff' <<< "$layout" &&
grep -Ei 'Flash Region 2 .*00003000 - 007fffff' <<< "$layout" &&
grep -Ei 'Flash Region 3 .*00001000 - 00002fff' <<< "$layout" &&
dd if="$rom" of="$out/descriptor.bin" bs=4096 count=1 status=none &&
dd if="$rom" of="$out/me.bin" bs=4096 skip=3 count=2045 status=none &&
dd if="$rom" of="$out/gbe.bin" bs=4096 skip=1 count=2 status=none &&
stat -c '%n: %s bytes' "$out"/{descriptor,me,gbe}.bin
```

Expected sizes are **4,096**, **8,376,320** and **8,192** bytes respectively. Use the target board's own GbE region, which can carry its MAC address.

### 2. Extract the NEC Intel GOP and matching VBT with UEFITool NE

1. Open `VX-2-original.rom` in [UEFITool NE](https://github.com/LongSoft/UEFITool/releases). Use **Search → GUID** and double-click the matching result to locate the firmware file in the tree.
2. Search for `FF0C8745-3270-4439-B74F-3E45F8C77064` (`IntelGopDriver`). Expand the file and any encapsulation sections. Right-click the **PE32 image section**, choose **Extract body…**, and save it in the staging directory as `IntelGopDriver.efi`.
3. Search for `878AC2CC-5343-46F2-B563-51F89DAF56BA` (`IntelGopVbt`). Right-click its **Raw section**, choose **Extract body…**, and save it there as `IntelGopVbt.bin`.
4. Check the resulting files. The tested GOP is a **69,568-byte x86-64 PE32+ EFI boot-service driver** beginning with `MZ`. The tested VBT is **4,608 bytes**, starts with `$VBT SKYLAKE`, and has the SHA-256 below. Keep the GOP and VBT from the same OEM image. If the GUIDs or formats differ, verify that firmware revision before using the files.

```bash
file "$out/IntelGopDriver.efi"
stat -c '%n: %s bytes' "$out"/IntelGop{Driver.efi,Vbt.bin}
sha256sum "$out/IntelGopVbt.bin"
# Tested VBT: 407ceb6c1a3eafafc9aa441207bc23d69728465ebc046997d300b7052da2ad33
```

### 3. Locate and validate the legacy Intel VGA option ROM

The published configuration uses the filename `pci8086,5917.rom` for the target iGPU. The 65,536-byte legacy image inspected in a test build reports **`8086:0406` inside its PCIR header**. No fixed OEM GUID or offset for this image has been verified. Search for the ROM structure below. If you cannot find a matching image in your OEM firmware, mark this input as unresolved. The GOP and the VBT are separate inputs.

First run the following scanner against the full dump. If it finds no candidate, use UEFITool NE to inspect the decompressed sections of the BIOS region. Search for the hex pattern `5043495286800604` (or `5043495286801759` for `8086:5917`). Extract the matching section's **body** to a temporary file and run the same scanner with that file as the first argument. Select an offset only after identifying the matching legacy VGA image. The scanner writes a file only when you pass an explicit candidate offset as the third argument.

```bash
# List candidates. To extract, add the printed offset after the output path.
python3 - "$rom" "$out/pci8086,5917.rom" <<'PY'
import pathlib
import re
import struct
import sys

data = pathlib.Path(sys.argv[1]).read_bytes()
output = pathlib.Path(sys.argv[2])
selected = int(sys.argv[3], 0) if len(sys.argv) > 3 else None
matches = {}
for match in re.finditer(b"\x55\xaa", data):
    start = match.start()
    if start + 0x1a > len(data):
        continue
    pcir = start + struct.unpack_from("<H", data, start + 0x18)[0]
    if pcir < start + 0x1a or pcir + 0x16 > len(data):
        continue
    if data[pcir:pcir + 4] != b"PCIR":
        continue
    vendor, device = struct.unpack_from("<HH", data, pcir + 4)
    size = struct.unpack_from("<H", data, pcir + 0x10)[0] * 512
    if (vendor != 0x8086 or device not in (0x0406, 0x5917)
            or data[pcir + 0x0d:pcir + 0x10] != b"\x00\x00\x03"
            or data[pcir + 0x14] != 0 or not size
            or not (pcir + 0x16 <= start + size <= len(data))):
        continue
    matches[start] = size
    print(f"{start:#x}: Intel {vendor:04x}:{device:04x}, legacy VGA ROM, {size} bytes")

if selected is not None:
    if selected not in matches:
        sys.exit("Selected offset is not a validated candidate.")
    with output.open("xb") as destination:
        destination.write(data[selected:selected + matches[selected]])
elif not matches:
    sys.exit("No candidate in this file. Check decompressed OEM sections.")
PY
```

For the tested **`8086:0406`** image, expect **65,536 bytes**. A different PCIR match is a candidate and still needs validation on the target board. A known-good local coreboot ROM that already contains the legacy image can also serve as a local fallback: `build/cbfstool /path/to/working.rom extract -n 'pci8086,5917.rom' -f "$out/pci8086,5917.rom"`.

### 4. Prepare the HAP descriptor

Keep `descriptor.bin` as the original-layout baseline. From a verified 16 MiB ROM with the target machine’s full ME and the original region boundaries, run the following in the coreboot checkout. Set `rom` to your own verified input and use a new output path.

```sh
rom="$PWD/VX-2-original.rom"
hap_image="$out/vx2-me-hap-only.rom"
desc="$out/descriptor_hap.bin"

test "$(stat -c %s "$rom")" = 16777216 &&
test ! -e "$hap_image" &&
python3 util/me_cleaner/me_cleaner.py -s -O "$hap_image" "$rom" &&
( set -C; head -c 4096 "$hap_image" > "$desc" ) &&
build/util/ifdtool/ifdtool -p sklkbl -d "$desc" |
  grep -Ei 'Flash Region [0-3] |HAP bit'
```

Check that HAP is set and the ME and BIOS boundaries match the original descriptor. The HAP build uses the full `me.bin` and disables build-time ME cleaning. See the [test matrix](test-matrix.md) for the verified boot result.

### 5. Stage the six inputs

After checking all six inputs, copy them into the board blob directory. The interactive copy prompts before replacing an existing file.

```bash
dest=3rdparty/blobs/mainboard/nec/vx-2
mkdir -p "$dest" &&
cp -i -- "$out"/{descriptor_hap.bin,me.bin,gbe.bin,IntelGopDriver.efi,IntelGopVbt.bin} "$out/pci8086,5917.rom" "$dest/"
```

Keep OEM binaries and dumps local. Build using the 16 MiB configuration with an 8 MiB BIOS area. Verify the generated ROM's descriptor and FMAP before flashing.

Avoid committing OEM flash dumps, Intel ME partitions, EC images, private keys, serial numbers, MAC addresses, UUIDs or NVRAM variables.
