/* SPDX-License-Identifier: GPL-2.0-only */

Scope (\_SB.PCI0.GFX0)
{
	External (NORB, IntObj)
	External (NMBX, FieldUnitObj)
	External (NSCC, FieldUnitObj)
	External (NSCP, FieldUnitObj)
	External (NDMI, FieldUnitObj)
	External (NBTD, FieldUnitObj)
	External (NBTV, FieldUnitObj)
	External (NPSC, FieldUnitObj)
	External (NPAT, FieldUnitObj)
	External (NBIA, FieldUnitObj)
	External (NPVL, FieldUnitObj)

	OperationRegion (NGPC, PCI_Config, 0xE8, 0x18)
	Field (NGPC, WordAcc, NoLock, Preserve)
	{
		NGSE, 1,
		, 14,
		NGSS, 1
	}
	Field (NGPC, DWordAcc, NoLock, Preserve)
	{
		Offset (0x14),
		NAAS, 32
	}

	Name (NDBT, Package ()
	{
		0, 0x07, 0x38, 0x01C0, 0x0E00, 0x3F, 0x01C7,
		0x0E07, 0x01F8, 0x0E38, 0x0FC0, 0, 0, 0, 0, 0,
		0x7000, 0x7007, 0x7038, 0x71C0, 0x7E00
	})

	/* Shared OEM boot-display encoding for _DSM 0x0D and GBDA 4. */
	Method (NDBP, 1, Serialized)
	{
		Local0 = NBTD
		If (Local0 >= SizeOf (NDBT))
		{
			Return (Buffer () { 0 })
		}
		Return ((Arg0 & 0xEFFF0000 & (DerefOf (NDBT [Local0]) << 16)) | Local0)
	}

	/* LIDS polarity follows the OEM panel _DCS and the physical HPLD bit. */
	Method (NLID, 0, Serialized)
	{
		If (!\_SB.PCI0.LPCB.EC.NERC)
		{
			Return (Buffer () { 0 })
		}
		Return (\_SB.PCI0.LPCB.EC.NHPL)
	}

	Mutex (NPLK, 0)

	Method (NSPP, 3, Serialized)
	{
		If (!CondRefOf (NPVL))
		{
			Return (Zero)
		}
		If (Acquire (NPLK, 0xFFFF))
		{
			Return (Zero)
		}
		NPSC = Arg0 & 0xFF
		Local0 = Arg1 & 0xFF
		If (Local0)
		{
			NPAT = Local0 - One
			NPVL = One
		}
		NBIA = Arg2 & 7
		Release (NPLK)
		Return (One)
	}

	Method (NPDT, 1, Serialized)
	{
		If (CondRefOf (NPVL))
		{
			If (NPVL && ObjectType (Arg0) == One)
			{
				If (!Acquire (NPLK, 0xFFFF))
				{
					Local0 = (NPSC | (NPAT << 8)) + 0x100
					Local0 |= Arg0 << 16
					Local0 += 0x10000
					Local0 |= NBIA << 20
					Release (NPLK)
					Return (Local0)
				}
			}
		}
		Return (Buffer () { 0 })
	}

	Method (_DSM, 4, Serialized)
	{
		If (Arg0 != ToUUID ("3e5b41c6-eb1d-4260-9d15-c71fbadae414") ||
		    Arg1 != One)
		{
			Return (Buffer () { 0 })
		}

		Switch (ToInteger (Arg2))
		{
			Case (Zero)
			{
				Local0 = 0x027D
				If (CondRefOf (NBTV))
				{
					Local0 |= 0x2180
				}
				If (\_SB.PCI0.LPCB.EC.NERC)
				{
					Local0 |= 0x0400
					If (CondRefOf (NPVL))
					{
						If (NPVL)
						{
							Local0 |= 0x4000
						}
					}
				}
#if CONFIG(NEC_VX2_IGD_DSM_INTEGER_QUERY)
				Return (Local0)
#else
				Local1 = Buffer (2) {}
				Local1 [Zero] = Local0 & 0xFF
				Local1 [One] = Local0 >> 8
				Return (Local1)
#endif
			}
			Case (0x02) { Return (One) }
			Case (0x03) { Return (One) }
			Case (0x04) { Return (One) }
			Case (0x05) { Return (One) }
			Case (0x06) { Return (One) }
			Case (0x09) { Return (One) }
			Case (0x07)
			{
				If (!CondRefOf (NBTV) ||
				    (ObjectType (Arg3) != 3 && ObjectType (Arg3) != 4))
				{
					Return (Buffer () { 0 })
				}
				If (SizeOf (Arg3) < One)
				{
					Return (Buffer () { 0 })
				}

				Local0 = DerefOf (Arg3 [Zero])
				If (ObjectType (Local0) != One)
				{
					Return (Buffer () { 0 })
				}
				Local0 &= 0xFF
				If (Local0 >= SizeOf (NDBT))
				{
					Return (Buffer () { 0 })
				}

				NBTD = Local0
				NBTV = One
				Return (One)
			}
			Case (0x08)
			{
				If (ObjectType (Arg3) != 3 && ObjectType (Arg3) != 4)
				{
					Return (Buffer () { 0 })
				}
				If (SizeOf (Arg3) < 3)
				{
					Return (Buffer () { 0 })
				}
				Local0 = DerefOf (Arg3 [Zero])
				Local1 = DerefOf (Arg3 [One])
				Local2 = DerefOf (Arg3 [2])
				If (ObjectType (Local0) != One || ObjectType (Local1) != One ||
				    ObjectType (Local2) != One)
				{
					Return (Buffer () { 0 })
				}
				If (NSPP (Local0, Local1, Local2 >> 4))
				{
					Return (One)
				}
			}
			Case (0x0A)
			{
				Local0 = NLID ()
				If (ObjectType (Local0) == One)
				{
					Return ((Local0 << 8) + 0x100)
				}
			}
			Case (0x0E)
			{
				Return (NPDT (NLID ()))
			}
			Case (0x0D)
			{
				If (!CondRefOf (NBTV))
				{
					Return (Buffer () { 0 })
				}
				If (!NBTV || (ObjectType (Arg3) != 3 && ObjectType (Arg3) != 4))
				{
					Return (Buffer () { 0 })
				}
				If (SizeOf (Arg3) < 4)
				{
					Return (Buffer () { 0 })
				}

				Local0 = DerefOf (Arg3 [2])
				Local1 = DerefOf (Arg3 [3])
				If (ObjectType (Local0) != One || ObjectType (Local1) != One)
				{
					Return (Buffer () { 0 })
				}

				Return (NDBP (((Local0 & 0xFF) << 16) | ((Local1 & 0xFF) << 24)))
			}
		}

		Return (Buffer () { 0 })
	}

	/* Return SCIC exit status. PARM is written before completion. */
	Method (GBDA, 1, Serialized)
	{
		Local0 = NSCP
		NSCP = Zero
		Switch (ToInteger (Arg0))
		{
			Case (Zero)
			{
				NSCP = One
				If (CondRefOf (NBTV))
				{
					If (NBTV)
					{
						NSCP |= 0x08
					}
				}
				If (CondRefOf (NPVL))
				{
					If (NPVL && \_SB.PCI0.LPCB.EC.NERC)
					{
						NSCP |= 0x10
					}
				}
				Return (One)
			}
			Case (One) { Return (One) }
			Case (5)
			{
				Local1 = NPDT (NLID ())
				If (ObjectType (Local1) == One)
				{
					NSCP = Local1
					Return (One)
				}
			}
			Case (4)
			{
				If (CondRefOf (NBTV))
				{
					If (NBTV)
					{
						Local1 = NDBP (Local0)
						If (ObjectType (Local1) == One)
						{
							NSCP = Local1
							Return (One)
						}
					}
				}
			}
		}
		Return (2)
	}

	Method (SBCB, 1, Serialized)
	{
		Local0 = NSCP
		NSCP = Zero
		Switch (ToInteger (Arg0))
		{
			Case (Zero)
			{
				NSCP = 0x9D
				If (CondRefOf (NPVL))
				{
					NSCP |= 0x200
				}
				If (\_SB.PCI0.LPCB.EC.NERC)
				{
					NSCP |= 0x10000
				}
				Return (One)
			}
			Case (One) { Return (One) }
			Case (3) { Return (One) }
			Case (4) { Return (One) }
			Case (5) { Return (One) }
			Case (8) { Return (One) }
			Case (0x0A)
			{
				If (NSPP (Local0, Local0 >> 8, Local0 >> 20))
				{
					Return (One)
				}
			}
			Case (0x11)
			{
				Local1 = NLID ()
				If (ObjectType (Local1) == One)
				{
					NSCP = (Local1 << 8) + 0x100
					Return (One)
				}
			}
		}
		Return (2)
	}

	Method (GSCI, 0, Serialized)
	{
		If (!CondRefOf (NORB))
		{
			Return (Zero)
		}
		If (NAAS != NORB)
		{
			Return (Zero)
		}
		/* AML logical operators can evaluate both operands. */
		If (!(NMBX & 2))
		{
			Return (Zero)
		}
		If (!NDMI)
		{
			Return (Zero)
		}

		/* TCO SCI is shared. Leave unrelated events for their owner. */
		If (!NGSE || !NGSS)
		{
			Return (Zero)
		}

		Local0 = NSCC
		If (Local0 & One)
		{
			Local1 = (Local0 >> One) & 0x0F
			Local2 = (Local0 >> 8) & 0xFF
			Local3 = 2
			If (Local1 == 4)
			{
				Local3 = GBDA (Local2)
			}
			ElseIf (Local1 == 6)
			{
				Local3 = SBCB (Local2)
			}
			Else
			{
				NSCP = Zero
			}

			/* End the hardware request before publishing completion. */
			NGSE = Zero
			NDMI = One
			NSCC = Local3 << 5
		}
		Else
		{
			NGSE = Zero
			NDMI = One
		}
		Return (One)
	}
}

/* Keep the candidate unregistered while mailbox 2 is off. */
Scope (\_GPE)
{
	Method (NG66, 0, NotSerialized)
	{
		\_SB.PCI0.GFX0.GSCI ()
	}
}
