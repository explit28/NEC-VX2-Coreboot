#!/bin/bash
set -eo pipefail
# Set VX2_EDK2_WORKSPACE to the parent of mrchromebox and edk2-platforms.
export WORKSPACE="$(cd "${VX2_EDK2_WORKSPACE:?Set VX2_EDK2_WORKSPACE}" && pwd)"
export EDK2_PATH="$WORKSPACE/mrchromebox"
export PACKAGES_PATH="$EDK2_PATH"
for part in Platform/Intel Silicon/Intel Features/Intel Features/Intel/{Debugging,Network,OutOfBandManagement,PowerManagement,SystemInformation,UserInterface}; do
  PACKAGES_PATH+=":$WORKSPACE/edk2-platforms/$part"
done
cd "$WORKSPACE"
build_args=("$@")
set --
source "$EDK2_PATH/edksetup.sh"
build -q -a IA32 -a X64 -b RELEASE -t GCC -n 12 -p UefiPayloadPkg/UefiPayloadPkg.dsc \
  -D BOOTLOADER=COREBOOT -D BUILD_ARCH=X64 -D USE_EDK2_PLATFORMS=TRUE \
  -D BOOTSPLASH_IMAGE=TRUE -D BOOT_MANAGER_ESCAPE=TRUE -D SERIAL_DRIVER_ENABLE=FALSE \
  -D DISABLE_SERIAL_TERMINAL=TRUE -D VARIABLE_SUPPORT=SMMSTORE -D PS2_KEYBOARD_ENABLE=TRUE \
  -D SIO_BUS_ENABLE=TRUE -D PLATFORM_BOOT_TIMEOUT=2 -D SD_MMC_TIMEOUT=10000 \
  -D SECURE_BOOT_ENABLE=TRUE -D USE_PLATFORM_GOP=TRUE -D PRIORITIZE_INTERNAL=TRUE \
  -D LOAD_OPTION_ROMS=TRUE -D NETWORK_IPXE=TRUE -D LVGL_ENABLE=TRUE \
  -D NEC_VX2_PANEL_DXE=TRUE -D NEC_VX2_DIAGNOSTIC=TRUE \
  --pcd gUefiCpuPkgTokenSpaceGuid.PcdCpuCoreCrystalClockFrequency=24000000 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize=0x8000 \
  --pcd gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress=0xe0000000 \
  --pcd gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseSize=0x10000000 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow=0 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn=0 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow=0 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn=0 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId=COREv4 \
  --pcd gUefiCpuPkgTokenSpaceGuid.PcdFirstTimeWakeUpAPsBySipi=FALSE \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosVersion=0x0300 \
  --pcd gEfiMdeModulePkgTokenSpaceGuid.PcdSmbiosDocRev=0x0 "${build_args[@]}"
