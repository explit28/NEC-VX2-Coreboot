# Build Combined V2 and the NEC VX-2 IGD SID fix

The public configuration omits the test unit's SMBIOS serial number. Use a
fresh checkout for each build so that any existing ROMs remain available.
The hardware-tested Combined V2 source is revision
`a4459120ea33e06d6478eec48fe3f2d1650b0810`. Later revisions include the
board-specific SID correction.

## Prepare coreboot

Use a Linux host with Git, GNU make, GCC/G++, GNAT, flex, bison, makeinfo,
libelf, zlib, OpenSSL and UUID development libraries, Python 3, NASM,
ACPICA iASL and ImageMagick. EDK II also needs its Python build dependencies.
Build the coreboot cross toolchain with the compilers selected by this revision.

```sh
git clone https://github.com/explit28/NEC-VX2-Coreboot.git vx2-port
git clone https://review.coreboot.org/coreboot.git coreboot-vx2
export VX2_PORT="$PWD/vx2-port"
export VX2_COREBOOT="$PWD/coreboot-vx2"
cd "$VX2_COREBOOT"
git checkout --detach 8b0e34f90e08547cf19e550a43ed72f69c39d6ed
git submodule update --init
git submodule update --init --checkout 3rdparty/blobs 3rdparty/fsp 3rdparty/intel-microcode
mkdir -p src/mainboard/nec/versapro_vx2
cp "$VX2_PORT"/vendor-port/Kconfig* src/mainboard/nec/
cp -R "$VX2_PORT"/board-port/. src/mainboard/nec/versapro_vx2/
for patch in nec-vx2-cardbus-init-and-sd-bus coreboot-graphics coreboot-igd-nvs; do
    git apply "$VX2_PORT/patches/$patch.patch"
done
git -C 3rdparty/libgfxinit apply "$VX2_PORT/patches/libgfxinit-display-probing.patch"
cp "$VX2_PORT/config/coreboot.config" .config
make crossgcc-i386 CPUS="$(nproc)"
make crossgcc-x64 CPUS="$(nproc)"
make olddefconfig
```

Prepare the target machine's descriptor, full Intel ME, GbE, OEM GOP,
OEM VBT and legacy option ROM in the paths listed in [OEM assets](oem-assets.md).
The firmware inputs are supplied locally. The coreboot wrapper builds the
Secure Boot iPXE binary from its pinned revision
`7c39c04a537ce29dccc6f2bae9749d1d371429c1`.

```sh
make -j"$(nproc)" ipxe
```

## Build EDK II and the diagnostic

```sh
export VX2_EDK2_WORKSPACE="$VX2_COREBOOT/vx2-edk2-workspace"
mkdir -p "$VX2_EDK2_WORKSPACE"
cd "$VX2_EDK2_WORKSPACE"
git clone https://github.com/mrchromebox/edk2.git mrchromebox
git -C mrchromebox checkout --detach 57642656c3ddc5d55aeb2e8c3fb86c81276da4a7
git -C mrchromebox submodule update --init --recursive
git clone https://github.com/tianocore/edk2-platforms.git edk2-platforms
git -C edk2-platforms checkout --detach 4db2845d14425599238be86b8a73d288f7c0b4a6
git -C edk2-platforms submodule update --init --recursive
git -C mrchromebox apply --check "$VX2_PORT/edk2/NEC-VX2-IGD-Combined-v2-EDK2.patch"
git -C mrchromebox apply "$VX2_PORT/edk2/NEC-VX2-IGD-Combined-v2-EDK2.patch"
make -C mrchromebox/BaseTools -j"$(nproc)"

assets="$VX2_COREBOOT/3rdparty/blobs/mainboard/nec/vx-2"
package="$VX2_EDK2_WORKSPACE/mrchromebox/UefiPayloadPkg"
cp "$assets/IntelGopDriver.efi" "$package/"
cp "$assets/IntelGopVbt.bin" "$package/vbt.bin"
mkdir -p "$package/NetworkDrivers"
cp "$VX2_COREBOOT/payloads/external/iPXE/ipxe/ipxe.rom" "$package/NetworkDrivers/ipxe.efi"
convert -background None "$VX2_COREBOOT/Documentation/coreboot_logo.bmp" \
    "BMP3:$VX2_EDK2_WORKSPACE/mrchromebox/MdeModulePkg/Logo/Logo.bmp"
bash "$VX2_PORT/edk2/diagnostic/build-edk2.sh"
bash "$VX2_PORT/edk2/diagnostic/build-edk2.sh" -m UefiPayloadPkg/Vx2Igd/Vx2Diagnostic.inf
```

The application is under `Build/UefiPayloadPkgX64/RELEASE_GCC/X64` in the
EDK II workspace. Run `Vx2Diagnostic.efi` from a writable FAT volume in
the UEFI Shell. It saves the first free numbered report on that volume.
The default run reads hardware state. The optional `--edid-test` mode
temporarily writes the in-memory EDID handoff and restores it.

## Assemble and inspect the ROM

```sh
cd "$VX2_COREBOOT"
mkdir -p build
cp "$VX2_EDK2_WORKSPACE/Build/UefiPayloadPkgX64/RELEASE_GCC/FV/UEFIPAYLOAD.fd" build/
make -j"$(nproc)" -o build/UEFIPAYLOAD.fd
build/cbfstool build/coreboot.rom layout -w
build/cbfstool build/coreboot.rom print
build/util/ifdtool/ifdtool -p sklkbl -t build/coreboot.rom
sha256sum build/coreboot.rom
```

The `-o` option keeps coreboot's generic payload rule from rebuilding the
custom EDK II image. The result depends on the local OEM inputs, build
metadata and host tool versions. The expected ROM size is 16 MiB.
