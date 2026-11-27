// SPDX-License-Identifier: BSD-2-Clause-Patent
/** @file NEC VX-2 IGD handoff helpers. **/
#include "Vx2Igd.h"

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>

EFI_STATUS
Vx2MemoryMap (EFI_MEMORY_DESCRIPTOR **Map, UINTN *Size, UINTN *Stride, UINT32 *Version)
{
  EFI_STATUS Status;
  UINTN Key;
  UINTN Attempt;

  *Map = NULL;
  *Size = 0;
  for (Attempt = 0; Attempt < 5; ++Attempt) {
    Status = gBS->GetMemoryMap (Size, *Map, &Key, Stride, Version);
    if (!EFI_ERROR (Status)) {
      return Status;
    }
    if (*Map != NULL) {
      FreePool (*Map);
      *Map = NULL;
    }
    if (Status != EFI_BUFFER_TOO_SMALL) {
      return Status;
    }
    *Size += 8 * sizeof (EFI_MEMORY_DESCRIPTOR);
    *Map = AllocatePool (*Size);
    if (*Map == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  FreePool (*Map);
  *Map = NULL;
  return EFI_BUFFER_TOO_SMALL;
}

EFI_STATUS
Vx2FindPci (UINTN Device, EFI_HANDLE *Handle, EFI_PCI_IO_PROTOCOL **Pci)
{
  EFI_HANDLE *Handles;
  UINTN Count;
  UINTN Index;
  UINTN Segment, Bus, Slot, Function;
  EFI_PCI_IO_PROTOCOL *Candidate;
  UINT32 Id;
  EFI_STATUS Status;

  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPciIoProtocolGuid, NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < Count; ++Index) {
    if (EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiPciIoProtocolGuid,
                                      (VOID **)&Candidate)) ||
        EFI_ERROR (Candidate->GetLocation (Candidate, &Segment, &Bus, &Slot, &Function)) ||
        Segment != 0 || Bus != 0 || Slot != Device || Function != 0 ||
        EFI_ERROR (Candidate->Pci.Read (Candidate, EfiPciIoWidthUint32, 0, 1, &Id)) ||
        (UINT16)Id != 0x8086 || (Device == 2 && (Id >> 16) != 0x5917)) {
      continue;
    }
    *Handle = Handles[Index];
    *Pci = Candidate;
    Status = EFI_SUCCESS;
    break;
  }
  FreePool (Handles);
  return Status;
}

EFI_STATUS
Vx2OpRegion (EFI_PCI_IO_PROTOCOL *Pci, BOOLEAN Write, UINT8 **Region, UINTN *Size)
{
  EFI_MEMORY_DESCRIPTOR *Map;
  EFI_MEMORY_DESCRIPTOR *Entry;
  UINTN MapSize, Stride, Offset;
  UINT32 Version, Base, Length;
  UINT64 Remaining;
  UINT8 *Address;
  EFI_STATUS Status;

  Status = Pci->Pci.Read (Pci, EfiPciIoWidthUint32, 0xfc, 1, &Base);
  if (EFI_ERROR (Status) || Base == 0 || Base == MAX_UINT32 || (Base & 0xfff) != 0) {
    return EFI_NOT_FOUND;
  }
  Status = Vx2MemoryMap (&Map, &MapSize, &Stride, &Version);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Remaining = 0;
  for (Offset = 0; Offset < MapSize; Offset += Stride) {
    Entry = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Map + Offset);
    if (Entry->Type != EfiACPIMemoryNVS &&
        (Write || Entry->Type != EfiReservedMemoryType)) {
      continue;
    }
    if (Entry->Type == EfiReservedMemoryType && (Entry->Attribute & EFI_MEMORY_WB) == 0) {
      continue;
    }
    if ((Entry->Attribute & EFI_MEMORY_RP) != 0 ||
        (Write && (Entry->Attribute & (EFI_MEMORY_RO | EFI_MEMORY_WP)) != 0)) {
      continue;
    }
    if (Base >= Entry->PhysicalStart &&
        Base - Entry->PhysicalStart < EFI_PAGES_TO_SIZE (Entry->NumberOfPages)) {
      Remaining = EFI_PAGES_TO_SIZE (Entry->NumberOfPages) - (Base - Entry->PhysicalStart);
      break;
    }
  }
  FreePool (Map);
  if (Remaining < 8192) {
    return EFI_ACCESS_DENIED;
  }

  Address = (UINT8 *)(UINTN)Base;
  if (CompareMem (Address, "IntelGraphicsMem", 16) != 0) {
    return EFI_COMPROMISED_DATA;
  }
  Length = ReadUnaligned32 ((UINT32 *)(Address + 16));
  if (Length < 8 || Length > 64 || (UINT64)Length * 1024 > Remaining ||
      (UINT64)Base + Length * 1024 > BASE_4GB) {
    return EFI_COMPROMISED_DATA;
  }
  if (Write && (Length != 8 || Address[23] != 2 || Address[22] > 1 ||
                (Address[88] & BIT4) == 0)) {
    return EFI_UNSUPPORTED;
  }
  *Region = Address;
  *Size = Length * 1024;
  return EFI_SUCCESS;
}

STATIC BOOLEAN
ChecksumOk (CONST UINT8 *Block)
{
  UINT8 Sum = 0;
  UINTN Index;

  for (Index = 0; Index < 128; ++Index) {
    Sum = (UINT8)(Sum + Block[Index]);
  }
  return Sum == 0;
}

BOOLEAN
Vx2ValidEdid (CONST UINT8 *Edid, UINTN Size)
{
  STATIC CONST UINT8 Signature[] = { 0, 255, 255, 255, 255, 255, 255, 0 };

  return Edid != NULL && Size >= 128 &&
         CompareMem (Edid, Signature, sizeof Signature) == 0 && ChecksumOk (Edid);
}

STATIC UINTN
EdidSizeForMailbox (CONST UINT8 *Edid, UINTN Size)
{
  if (!Vx2ValidEdid (Edid, Size)) {
    return 0;
  }
  if (Edid[126] == 0) {
    return 128;
  }
  if (Edid[126] == 1 && Size >= 256 && ChecksumOk (Edid + 128)) {
    return 256;
  }
  return 0;
}

STATIC BOOLEAN
IsPanelChild (EFI_HANDLE Handle, EFI_DEVICE_PATH_PROTOCOL *Parent, UINTN Prefix)
{
  EFI_DEVICE_PATH_PROTOCOL *Path;
  EFI_DEVICE_PATH_PROTOCOL *Node;
  UINTN Size;
  UINT32 Adr;

  Path = DevicePathFromHandle (Handle);
  if (Path == NULL || Parent == NULL) {
    return FALSE;
  }
  Size = GetDevicePathSize (Path);
  if (Size != Prefix + sizeof (ACPI_ADR_DEVICE_PATH) + END_DEVICE_PATH_LENGTH ||
      CompareMem (Path, Parent, Prefix) != 0) {
    return FALSE;
  }
  Node = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8 *)Path + Prefix);
  if (DevicePathType (Node) != ACPI_DEVICE_PATH ||
      DevicePathSubType (Node) != ACPI_ADR_DP ||
      DevicePathNodeLength (Node) != sizeof (ACPI_ADR_DEVICE_PATH) ||
      !IsDevicePathEnd (NextDevicePathNode (Node))) {
    return FALSE;
  }
  Adr = ReadUnaligned32 (&((ACPI_ADR_DEVICE_PATH *)Node)->ADR);
  return ((Adr >> 8) & 0xf) == ACPI_ADR_DISPLAY_TYPE_INTERNAL_DIGITAL;
}

EFI_STATUS
Vx2PanelEdid (EFI_HANDLE Igd, UINT8 Edid[256], UINTN *Size)
{
  EFI_DEVICE_PATH_PROTOCOL *Parent;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  EFI_EDID_DISCOVERED_PROTOCOL *Discovered;
  EFI_EDID_ACTIVE_PROTOCOL *Active;
  EFI_HANDLE *Handles;
  CONST UINT8 *Chosen;
  UINTN ActiveSize, DiscoveredSize, Count, Index, Prefix, Internal;
  EFI_STATUS Status;

  *Size = 0;
  Parent = DevicePathFromHandle (Igd);
  if (Parent == NULL) {
    return EFI_NOT_FOUND;
  }
  Prefix = GetDevicePathSize (Parent) - END_DEVICE_PATH_LENGTH;
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiGraphicsOutputProtocolGuid,
                                    NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Internal = 0;
  for (Index = 0; Index < Count; ++Index) {
    if (!IsPanelChild (Handles[Index], Parent, Prefix)) {
      continue;
    }
    ++Internal;
    if (EFI_ERROR (gBS->HandleProtocol (Handles[Index], &gEfiGraphicsOutputProtocolGuid,
                                      (VOID **)&Gop)) ||
        Gop->Mode == NULL || Gop->Mode->Info == NULL || Gop->Mode->Mode >= Gop->Mode->MaxMode ||
        Gop->Mode->Info->HorizontalResolution == 0 || Gop->Mode->Info->VerticalResolution == 0) {
      continue;
    }
    Discovered = NULL;
    Active = NULL;
    gBS->HandleProtocol (Handles[Index], &gEfiEdidDiscoveredProtocolGuid, (VOID **)&Discovered);
    gBS->HandleProtocol (Handles[Index], &gEfiEdidActiveProtocolGuid, (VOID **)&Active);
    DiscoveredSize = Discovered == NULL ? 0 :
                     EdidSizeForMailbox (Discovered->Edid, Discovered->SizeOfEdid);
    ActiveSize = Active == NULL ? 0 : EdidSizeForMailbox (Active->Edid, Active->SizeOfEdid);
    // Reject ambiguity between physical and active panel identities.
    if (DiscoveredSize != 0 && ActiveSize != 0 &&
        CompareMem (Discovered->Edid + 8, Active->Edid + 8, 4) != 0) {
      continue;
    }
    if (DiscoveredSize != 0) {
      Chosen = Discovered->Edid;
      *Size = DiscoveredSize;
    } else if (ActiveSize != 0) {
      Chosen = Active->Edid;
      *Size = ActiveSize;
    } else {
      continue;
    }
    CopyMem (Edid, Chosen, *Size);
  }
  FreePool (Handles);
  return Internal == 1 && *Size != 0 ? EFI_SUCCESS : EFI_NOT_FOUND;
}

VOID
Vx2PublishEdid (UINT8 *Region, CONST UINT8 *Edid, UINTN Size)
{
  WriteUnaligned32 ((UINT32 *)(Region + VX2_PHED), 0);
  ZeroMem (Region + VX2_BDDC, 256);
  CopyMem (Region + VX2_BDDC, Edid, Size);
  MemoryFence ();
  WriteUnaligned32 ((UINT32 *)(Region + VX2_PHED), (UINT32)(Size / 128));
}
