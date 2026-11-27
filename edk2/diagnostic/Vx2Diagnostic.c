// SPDX-License-Identifier: BSD-2-Clause-Patent
/** @file Read-only NEC VX-2 graphics snapshot, with an explicit RAM test. **/
#include "Vx2Igd.h"

#include <IndustryStandard/Acpi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/SimpleFileSystem.h>

STATIC EFI_FILE_PROTOCOL *mFile;
STATIC EFI_STATUS mFileStatus;

STATIC VOID EFIAPI
Log (CONST CHAR8 *Format, ...)
{
  CHAR8 Line[512];
  VA_LIST Args;
  UINTN Length, Written;

  if (EFI_ERROR (mFileStatus)) {
    return;
  }
  VA_START (Args, Format);
  Length = AsciiVSPrint (Line, sizeof Line, Format, Args);
  VA_END (Args);
  Written = Length;
  mFileStatus = mFile->Write (mFile, &Written, Line);
  if (!EFI_ERROR (mFileStatus) && Written != Length) {
    mFileStatus = EFI_DEVICE_ERROR;
  }
}

STATIC VOID
HexDump (CONST UINT8 *Data, UINTN Size)
{
  UINTN Offset, Index, Pos;
  CHAR8 Line[80];

  for (Offset = 0; Offset < Size; Offset += 16) {
    Pos = AsciiSPrint (Line, sizeof Line, "%04x", Offset);
    for (Index = Offset; Index < Size && Index < Offset + 16; ++Index) {
      Pos += AsciiSPrint (Line + Pos, sizeof Line - Pos, " %02x", Data[Index]);
    }
    Log ("%a\r\n", Line);
  }
}

STATIC VOID
DumpPci (UINTN Slot)
{
  EFI_HANDLE Handle;
  EFI_PCI_IO_PROTOCOL *Pci;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *Root;
  UINT8 Data[256];
  UINTN Offset;
  EFI_STATUS Status;

  Status = Vx2FindPci (Slot, &Handle, &Pci);
  Root = NULL;
  if (EFI_ERROR (Status)) {
    if (Slot != 0 ||
        EFI_ERROR (gBS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL, (VOID **)&Root)) ||
        Root->SegmentNumber != 0) {
      Log ("PCI 00:%02x.0 unavailable: %r\r\n", Slot, Status);
      return;
    }
  }

  for (Offset = 0; Offset < 4096; Offset += sizeof Data) {
    if (Root != NULL) {
      Status = Root->Pci.Read (Root, EfiPciWidthUint32,
                 EFI_PCI_ADDRESS (0, Slot, 0, Offset), sizeof Data / 4, Data);
    } else {
      Status = Pci->Pci.Read (Pci, EfiPciIoWidthUint32, (UINT32)Offset, sizeof Data / 4, Data);
    }
    if (EFI_ERROR (Status)) {
      Log ("PCI 00:%02x.0 offset %04x unavailable: %r\r\n", Slot, Offset, Status);
      break;
    }
    Log ("PCI 00:%02x.0 offset %04x\r\n", Slot, Offset);
    HexDump (Data, sizeof Data);
    if (Offset != 0) {
      continue;
    }
    Log ("COMMAND=%04x BME=%u GGC=%04x\r\n",
         ReadUnaligned16 ((UINT16 *)(Data + 4)), (Data[4] & BIT2) != 0,
         ReadUnaligned16 ((UINT16 *)(Data + 0x50)));
    if (Slot == 0) {
      Log ("PAVPC=%08x BDSM=%08x BGSM=%08x TOLUD=%08x\r\n",
           ReadUnaligned32 ((UINT32 *)(Data + 0x58)),
           ReadUnaligned32 ((UINT32 *)(Data + 0xb0)),
           ReadUnaligned32 ((UINT32 *)(Data + 0xb4)),
           ReadUnaligned32 ((UINT32 *)(Data + 0xbc)));
    } else {
      Log ("MSAC=%02x SSVID=%04x SSID=%04x BDSM=%08x ASLS=%08x SWSCI=%04x\r\n",
           Data[0x62], ReadUnaligned16 ((UINT16 *)(Data + 0x2c)),
           ReadUnaligned16 ((UINT16 *)(Data + 0x2e)), ReadUnaligned32 ((UINT32 *)(Data + 0x5c)),
           ReadUnaligned32 ((UINT32 *)(Data + 0xfc)), ReadUnaligned16 ((UINT16 *)(Data + 0xe8)));
    }
  }
}

STATIC VOID
DumpEdid (CONST CHAR8 *Name, CONST UINT8 *Edid, UINTN Size)
{
  Log ("%a size=%u base-check=%u\r\n", Name, Size, Vx2ValidEdid (Edid, Size));
  if (Edid != NULL && Size <= 32768) {
    HexDump (Edid, Size);
  }
}

STATIC VOID
DumpDisplays (VOID)
{
  EFI_HANDLE *Handles;
  UINTN Count, Index;
  EFI_STATUS Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  EFI_EDID_DISCOVERED_PROTOCOL *Discovered;
  EFI_EDID_ACTIVE_PROTOCOL *Active;
  EFI_DEVICE_PATH_PROTOCOL *Path;
  CHAR16 *PathText;

  Status = gBS->LocateHandleBuffer (AllHandles, NULL, NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) {
    Log ("Display enumeration: %r\r\n", Status);
    return;
  }
  for (Index = 0; Index < Count; ++Index) {
    Gop = NULL;
    Discovered = NULL;
    Active = NULL;
    gBS->HandleProtocol (Handles[Index], &gEfiGraphicsOutputProtocolGuid, (VOID **)&Gop);
    gBS->HandleProtocol (Handles[Index], &gEfiEdidDiscoveredProtocolGuid, (VOID **)&Discovered);
    gBS->HandleProtocol (Handles[Index], &gEfiEdidActiveProtocolGuid, (VOID **)&Active);
    if (Gop == NULL && Discovered == NULL && Active == NULL) {
      continue;
    }
    Path = DevicePathFromHandle (Handles[Index]);
    if (Path != NULL) {
      PathText = ConvertDevicePathToText (Path, FALSE, FALSE);
      if (PathText != NULL) {
        Log ("Display handle %u %s\r\n", Index, PathText);
        FreePool (PathText);
      }
      HexDump ((UINT8 *)Path, GetDevicePathSize (Path));
    } else {
      Log ("Display handle %u: no device path\r\n", Index);
    }
    if (Gop != NULL && Gop->Mode != NULL && Gop->Mode->Info != NULL) {
      Log ("GOP mode=%u/%u info-version=%u %ux%u stride=%u format=%u fb=%016Lx size=%Lx\r\n",
           Gop->Mode->Mode, Gop->Mode->MaxMode, Gop->Mode->Info->Version,
           Gop->Mode->Info->HorizontalResolution, Gop->Mode->Info->VerticalResolution,
           Gop->Mode->Info->PixelsPerScanLine, Gop->Mode->Info->PixelFormat,
           Gop->Mode->FrameBufferBase, (UINT64)Gop->Mode->FrameBufferSize);
      Log ("GOP masks R=%08x G=%08x B=%08x reserved=%08x\r\n",
           Gop->Mode->Info->PixelInformation.RedMask, Gop->Mode->Info->PixelInformation.GreenMask,
           Gop->Mode->Info->PixelInformation.BlueMask,
           Gop->Mode->Info->PixelInformation.ReservedMask);
    }
    if (Discovered != NULL) {
      DumpEdid ("EDID Discovered", Discovered->Edid, Discovered->SizeOfEdid);
    }
    if (Active != NULL) {
      DumpEdid ("EDID Active", Active->Edid, Active->SizeOfEdid);
    }
  }
  FreePool (Handles);
}

STATIC BOOLEAN
PciInD0 (EFI_PCI_IO_PROTOCOL *Pci)
{
  UINT8 Config[256];
  UINT8 Offset;
  UINTN Index;

  if (EFI_ERROR (Pci->Pci.Read (Pci, EfiPciIoWidthUint32, 0, sizeof Config / 4, Config))) {
    return FALSE;
  }
  if ((Config[6] & BIT4) == 0) {
    return FALSE;
  }
  Offset = Config[0x34] & 0xfc;
  for (Index = 0; Index < 48 && Offset >= 0x40 && Offset <= 0xf8; ++Index) {
    if (Config[Offset] == 1) {
      return (Config[Offset + 4] & 3) == 0;
    }
    Offset = Config[Offset + 1] & 0xfc;
  }
  return FALSE;
}

STATIC VOID
DumpMmio (EFI_PCI_IO_PROTOCOL *Pci)
{
  STATIC CONST struct {
    UINT32 Offset;
    CONST CHAR8 *Name;
  } Registers[] = {
    { 0x45400, "PWR_WELL_CTL BIOS" },
    { 0x45404, "PWR_WELL_CTL DRIVER" },
    { 0x45408, "PWR_WELL_CTL KVMR" },
    { 0x4540c, "PWR_WELL_CTL DEBUG" },
    { 0x42000, "FUSE_STATUS" },
    { 0x46000, "CDCLK_CTL" },
    { 0x1082c0, "STOLEN_RESERVED" }
  };
  UINT16 Command, Ggc;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *BarResource;
  UINT32 Bar, Value;
  UINTN Index;
  UINT64 Pte[8], GttSize, GttOffset, BarSize;
  EFI_STATUS Status;

  if (!PciInD0 (Pci) ||
      EFI_ERROR (Pci->Pci.Read (Pci, EfiPciIoWidthUint16, 4, 1, &Command)) ||
      (Command & BIT1) == 0 ||
      EFI_ERROR (Pci->Pci.Read (Pci, EfiPciIoWidthUint32, 0x10, 1, &Bar)) ||
      (Bar & BIT0) != 0 || Bar == MAX_UINT32 || (Bar & ~0xfU) == 0) {
    Log ("MMIO skipped: D0, memory decoding or BAR0 unavailable\r\n");
    return;
  }
  Status = Pci->GetBarAttributes (Pci, 0, NULL, (VOID **)&BarResource);
  if (EFI_ERROR (Status) || BarResource == NULL) {
    Log ("MMIO skipped: BAR0 resource unavailable: %r\r\n", Status);
    return;
  }
  BarSize = (BarResource->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR &&
             BarResource->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) ? BarResource->AddrLen : 0;
  FreePool (BarResource);
  if (BarSize != SIZE_16MB) {
    Log ("MMIO skipped: unexpected Kaby Lake BAR0 size %Lx\r\n", BarSize);
    return;
  }
  for (Index = 0; Index < ARRAY_SIZE (Registers); ++Index) {
    Status = Pci->Mem.Read (Pci, EfiPciIoWidthUint32, 0, Registers[Index].Offset, 1, &Value);
    if (EFI_ERROR (Status)) {
      Log ("%a BAR0+%x: %r\r\n", Registers[Index].Name, Registers[Index].Offset, Status);
    } else {
      Log ("%a BAR0+%x=%08x\r\n", Registers[Index].Name, Registers[Index].Offset, Value);
    }
  }
  if (EFI_ERROR (Pci->Pci.Read (Pci, EfiPciIoWidthUint16, 0x50, 1, &Ggc)) ||
      (Ggc & BIT1) != 0 || ((Ggc >> 6) & 3) == 0) {
    Log ("GGTT skipped: GGC does not describe an enabled GGTT\r\n");
    return;
  }

  // Gen9 GGTT PTEs occupy the second half of GTTMMADR.
  GttSize = LShiftU64 (1, 20 + ((Ggc >> 6) & 3));
  GttOffset = BarSize / 2;
  Status = Pci->Mem.Read (Pci, EfiPciIoWidthUint64, 0, GttOffset, ARRAY_SIZE (Pte), Pte);
  Log ("GGTT BAR0+%Lx size=%Lx first PTEs: %r\r\n", GttOffset, GttSize, Status);
  if (!EFI_ERROR (Status)) {
    HexDump ((UINT8 *)Pte, sizeof Pte);
  }
}

STATIC VOID
DumpMemoryMap (VOID)
{
  EFI_MEMORY_DESCRIPTOR *Map, *Entry;
  UINTN Size, Stride, Offset;
  UINT32 Version;
  EFI_STATUS Status;

  Status = Vx2MemoryMap (&Map, &Size, &Stride, &Version);
  Log ("EFI memory map: %r\r\n", Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  Log ("descriptor version=%u stride=%u bytes=%u\r\n", Version, Stride, Size);
  for (Offset = 0; Offset < Size; Offset += Stride) {
    Entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
    Log ("type=%02u physical=%016Lx virtual=%016Lx pages=%Lx attr=%016Lx\r\n", Entry->Type,
         Entry->PhysicalStart, Entry->VirtualStart, Entry->NumberOfPages, Entry->Attribute);
  }
  FreePool (Map);
}

STATIC EFI_STATUS
TestEdid (EFI_HANDLE Igd, EFI_PCI_IO_PROTOCOL *Pci)
{
  UINT8 Edid[256], *Region;
  UINT8 OldBddc[256];
  UINTN Size, RegionSize;
  UINT32 OldPhed;
  EFI_STATUS Status;

  Status = Vx2PanelEdid (Igd, Edid, &Size);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Vx2OpRegion (Pci, TRUE, &Region, &RegionSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  OldPhed = ReadUnaligned32 ((UINT32 *)(Region + VX2_PHED));
  CopyMem (OldBddc, Region + VX2_BDDC, sizeof OldBddc);
  Vx2PublishEdid (Region, Edid, Size);
  Status = (ReadUnaligned32 ((UINT32 *)(Region + VX2_PHED)) == Size / 128 &&
            CompareMem (Region + VX2_BDDC, Edid, Size) == 0) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
  WriteUnaligned32 ((UINT32 *)(Region + VX2_PHED), 0);
  CopyMem (Region + VX2_BDDC, OldBddc, sizeof OldBddc);
  MemoryFence ();
  WriteUnaligned32 ((UINT32 *)(Region + VX2_PHED), OldPhed);
  if (ReadUnaligned32 ((UINT32 *)(Region + VX2_PHED)) != OldPhed ||
      CompareMem (Region + VX2_BDDC, OldBddc, sizeof OldBddc) != 0) {
    return EFI_DEVICE_ERROR;
  }
  return Status;
}

EFI_STATUS EFIAPI
UefiMain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  EFI_SHELL_PARAMETERS_PROTOCOL *Args;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
  EFI_FILE_PROTOCOL *Root;
  EFI_HANDLE Igd;
  EFI_PCI_IO_PROTOCOL *Pci;
  EFI_STATUS Status, TestStatus;
  BOOLEAN Test;
  UINT8 *Region;
  UINTN Size, Index;
  CHAR16 Name[64];

  Test = FALSE;
  Status = gBS->HandleProtocol (ImageHandle, &gEfiShellParametersProtocolGuid, (VOID **)&Args);
  if (!EFI_ERROR (Status) && Args->Argc > 1) {
    if (Args->Argc != 2 || StrCmp (Args->Argv[1], L"--edid-test") != 0) {
      Print (L"Usage: NEC-VX2-IGD-Diagnostic.efi [--edid-test]\r\n");
      return EFI_INVALID_PARAMETER;
    }
    Test = TRUE;
  }

  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Image);
  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (Image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID **)&Fs);
  }
  if (!EFI_ERROR (Status)) {
    Status = Fs->OpenVolume (Fs, &Root);
  }
  if (EFI_ERROR (Status)) {
    Print (L"Cannot open the application's filesystem: %r\r\n", Status);
    return Status;
  }

  for (Index = 0; Index < 100; ++Index) {
    UnicodeSPrint (Name, sizeof Name, L"NEC-VX2-IGD-Diagnostic-%02u.txt", Index);
    Status = Root->Open (Root, &mFile, Name, EFI_FILE_MODE_READ, 0);
    if (Status == EFI_NOT_FOUND) {
      Status = Root->Open (Root, &mFile, Name,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
      break;
    }
    if (EFI_ERROR (Status)) {
      break;
    }
    mFile->Close (mFile);
    Status = EFI_VOLUME_FULL;
  }
  Root->Close (Root);
  if (EFI_ERROR (Status)) {
    Print (L"Cannot create report: %r\r\n", Status);
    return Status;
  }

  Log ("NEC VX-2 IGD Diagnostic v2\r\n");
  Log ("Firmware %s revision %08x\r\n", SystemTable->FirmwareVendor,
       SystemTable->FirmwareRevision);
  DumpPci (0);
  DumpPci (2);
  DumpDisplays ();
  DumpMemoryMap ();
  Status = Vx2FindPci (2, &Igd, &Pci);
  TestStatus = Status;
  if (!EFI_ERROR (Status)) {
    Status = Vx2OpRegion (Pci, FALSE, &Region, &Size);
    Log ("OpRegion: %r\r\n", Status);
    if (!EFI_ERROR (Status)) {
      Log ("OpRegion base=%016Lx bytes=%u version=%u.%u mailboxes=%08x\r\n",
           (UINT64)(UINTN)Region, Size, Region[23], Region[22],
           ReadUnaligned32 ((UINT32 *)(Region + 88)));
      Log ("GOP DVER (UTF-16LE):\r\n");
      HexDump (Region + 100, 32);
      HexDump (Region, Size);
    }
    DumpMmio (Pci);
    if (Test && !EFI_ERROR (mFileStatus)) {
      TestStatus = TestEdid (Igd, Pci);
    }
  } else {
    Log ("IGD 8086:5917 PCI I/O unavailable: %r\r\n", Status);
  }
  if (Test) {
    Log ("EDID temporary RAM write/read/restore test: %r\r\n", TestStatus);
  }
  Log ("End of report\r\n");
  Status = mFile->Close (mFile);
  if (!EFI_ERROR (mFileStatus)) {
    mFileStatus = Status;
  }
  Print (L"\\%s: %r\r\n", Name, mFileStatus);
  return Test && !EFI_ERROR (mFileStatus) ? TestStatus : mFileStatus;
}
