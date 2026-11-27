// SPDX-License-Identifier: BSD-2-Clause-Patent
/** @file Publish NEC panel data at ReadyToBoot, after console/GOP connection. **/
#include "Vx2Igd.h"

#include <Guid/EventGroup.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Vx2PanelProfiles.h"

#define EC_STATUS_PORT  0x66
#define EC_DATA_PORT    0x62
#define EC_OBF          BIT0
#define EC_IBF          BIT1
#define EC_BURST        BIT4

STATIC BOOLEAN
WaitEc (UINT8 Mask, UINT8 Expected)
{
  UINTN Attempt;
  UINT8 Status;

  for (Attempt = 0; Attempt < 200; ++Attempt) {
    Status = IoRead8 (EC_STATUS_PORT);
    if (Status == 0xff) {
      return FALSE;
    }
    if ((Status & Mask) == Expected) {
      return TRUE;
    }
    gBS->Stall (50);
  }
  return FALSE;
}

STATIC BOOLEAN
ReadLid (UINT32 *LidOpen)
{
  EFI_HANDLE Lpc;
  EFI_PCI_IO_PROTOCOL *Pci;
  UINT16 Decode;

  if (EFI_ERROR (Vx2FindPci (0x1f, &Lpc, &Pci)) ||
      EFI_ERROR (Pci->Pci.Read (Pci, EfiPciIoWidthUint16, 0x82, 1, &Decode)) ||
      (Decode & BIT11) == 0 || !WaitEc (EC_OBF | EC_IBF | EC_BURST, 0)) {
    return FALSE;
  }

  // OEM _LID and VideoInit both sample HPLD at EC offset 0x46, bit 2.
  IoWrite8 (EC_STATUS_PORT, 0x80);
  if (!WaitEc (EC_OBF | EC_IBF, 0)) {
    return FALSE;
  }
  IoWrite8 (EC_DATA_PORT, 0x46);
  if (!WaitEc (EC_OBF | EC_IBF, EC_OBF)) {
    return FALSE;
  }
  *LidOpen = (IoRead8 (EC_DATA_PORT) & BIT2) != 0;
  return TRUE;
}

STATIC VOID EFIAPI
Vx2AfterGop (EFI_EVENT Event, VOID *Context)
{
  EFI_HANDLE Igd;
  EFI_PCI_IO_PROTOCOL *Pci;
  UINT8 Edid[256];
  UINT8 *Region;
  UINTN EdidSize, RegionSize, Index, Curve;
  EFI_STATUS Status;
  CHAR16 GopVersion[17];
  UINT32 LidOpen;

  gBS->CloseEvent (Event);
  Status = Vx2FindPci (2, &Igd, &Pci);
  if (!EFI_ERROR (Status)) {
    Status = Vx2PanelEdid (Igd, Edid, &EdidSize);
  }
  if (!EFI_ERROR (Status)) {
    Status = Vx2OpRegion (Pci, TRUE, &Region, &RegionSize);
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "VX2 panel handoff skipped: %r\n", Status));
    return;
  }

  CopyMem (GopVersion, Region + 100, sizeof (GopVersion) - sizeof (CHAR16));
  GopVersion[16] = L'\0';
  for (Index = 0; GopVersion[Index] != L'\0'; ++Index) {
    if (GopVersion[Index] < L' ' || GopVersion[Index] > L'~') {
      DEBUG ((DEBUG_WARN, "VX2 invalid GOP DVER\n"));
      return;
    }
  }
  DEBUG ((DEBUG_INFO, "VX2 OpRegion %u.%u, GOP DVER %s, mask %02x\n",
          Region[23], Region[22], GopVersion[0] == 0 ? L"unavailable" : GopVersion,
          Region[88]));
  if ((Region[88] & BIT0) != 0 && ReadLid (&LidOpen)) {
    WriteUnaligned32 ((UINT32 *)(Region + 0x1ac), LidOpen);
  }
  // The OEM chooses BCLM by the final base-block descriptor. Publish live EDID.
  Curve = 2;
  for (Index = 0; Index < ARRAY_SIZE (mPanelKeys); ++Index) {
    if (CompareMem (Edid + 0x6c, mPanelKeys[Index], 18) == 0) {
      Curve = mPanelCurves[Index];
      break;
    }
  }

  if ((Region[88] & BIT2) != 0) {
    for (Index = 0; Index < 20; ++Index) {
      WriteUnaligned16 ((UINT16 *)(Region + 0x31c + 2 * Index),
                        (UINT16)(BIT15 | (mBrightnessPercent[Index] << 8) |
                                 mBrightnessDuty[Curve][Index]));
    }
    WriteUnaligned32 ((UINT32 *)(Region + 0x310), BIT31 | 255);
  }
  Vx2PublishEdid (Region, Edid, EdidSize);
}

EFI_STATUS EFIAPI
Vx2PanelEntry (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_EVENT Event;

  return gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, Vx2AfterGop,
                             NULL, &gEfiEventReadyToBootGuid, &Event);
}
