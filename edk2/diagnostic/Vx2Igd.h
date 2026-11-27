// SPDX-License-Identifier: BSD-2-Clause-Patent
/** @file NEC VX-2 IGD handoff helpers. **/
#pragma once

#include <Uefi.h>
#include <Protocol/PciIo.h>

#define VX2_PHED  0x1c00
#define VX2_BDDC  0x1c04

EFI_STATUS
Vx2MemoryMap (OUT EFI_MEMORY_DESCRIPTOR **Map, OUT UINTN *Size,
              OUT UINTN *Stride, OUT UINT32 *Version);

EFI_STATUS
Vx2FindPci (IN UINTN Device, OUT EFI_HANDLE *Handle, OUT EFI_PCI_IO_PROTOCOL **Pci);

EFI_STATUS
Vx2OpRegion (IN EFI_PCI_IO_PROTOCOL *Pci, IN BOOLEAN Write,
             OUT UINT8 **Region, OUT UINTN *Size);

EFI_STATUS
Vx2PanelEdid (IN EFI_HANDLE Igd, OUT UINT8 Edid[256], OUT UINTN *Size);

BOOLEAN
Vx2ValidEdid (IN CONST UINT8 *Edid, IN UINTN Size);

VOID
Vx2PublishEdid (IN OUT UINT8 *Region, IN CONST UINT8 *Edid, IN UINTN Size);
