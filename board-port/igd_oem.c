/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpigen.h>
#include <bootmem.h>
#include <cbmem.h>
#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ops.h>
#include <drivers/intel/gma/opregion.h>
#include <intelblocks/tco.h>
#include <soc/intel/common/tco.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct nec_igd_nvs {
	uint8_t boot_display;
	uint8_t boot_display_valid;
	uint8_t panel_scaling;
	uint8_t panel_type;
	uint8_t panel_bia;
	uint8_t panel_valid;
};

static const struct fieldlist nvs_fields[] = {
	FIELDLIST_NAMESTR("NBTD", 8),
	FIELDLIST_NAMESTR("NBTV", 8),
	FIELDLIST_NAMESTR("NPSC", 8),
	FIELDLIST_NAMESTR("NPAT", 8),
	FIELDLIST_NAMESTR("NBIA", 8),
	FIELDLIST_NAMESTR("NPVL", 8),
};

static void nec_igd_write_nvs(void)
{
	struct nec_igd_nvs *nvs = cbmem_find(CBMEM_ID_IGD_NVS);

	if (!nvs) {
		if (acpi_is_wakeup_s3())
			return;

		nvs = cbmem_add(CBMEM_ID_IGD_NVS, sizeof(*nvs));
		if (!nvs)
			return;

		memset(nvs, 0, sizeof(*nvs));
	}

	const struct opregion region = OPREGION("NGNV", SYSTEMMEMORY, (uintptr_t)nvs,
					       sizeof(*nvs));

	acpigen_write_opregion(&region);
	acpigen_write_field("NGNV", nvs_fields, ARRAY_SIZE(nvs_fields),
			    FIELD_BYTEACC | FIELD_NOLOCK | FIELD_PRESERVE);
}

static const struct fieldlist graphics_fields[] = {
	FIELDLIST_OFFSET(offsetof(igd_opregion_t, header.mailboxes)),
	FIELDLIST_NAMESTR("NMBX", 32),
	FIELDLIST_OFFSET(offsetof(igd_opregion_t, mailbox2.scic)),
	FIELDLIST_NAMESTR("NSCC", 32),
	FIELDLIST_NAMESTR("NSCP", 32),
};

static const struct fieldlist tco_fields[] = {
	FIELDLIST_RESERVED(9),
	FIELDLIST_NAMESTR("NDMI", 1),
};

static bool nec_igd_opregion_valid(const struct device *igd, const igd_opregion_t *opregion)
{
	const uintptr_t base = (uintptr_t)opregion;

	return igd && igd->enabled && base && IS_ALIGNED(base, 4 * KiB) &&
	       base <= UINT32_MAX - sizeof(*opregion) &&
	       pci_read_config32(igd, ASLS) == base &&
	       !memcmp(opregion->header.signature, IGD_OPREGION_SIGNATURE,
		       sizeof(opregion->header.signature)) &&
	       opregion->header.size == sizeof(*opregion) / KiB;
}

void bootmem_platform_add_ranges(void)
{
	const struct device *igd = pcidev_path_on_root(PCI_DEVFN(2, 0));
	const igd_opregion_t *opregion = cbmem_find(CBMEM_ID_IGD_OPREGION);

	if (nec_igd_opregion_valid(igd, opregion))
		bootmem_add_range_from((uintptr_t)opregion, sizeof(*opregion),
				       BM_MEM_NVS, BM_MEM_TABLE);
}

static void nec_igd_fill_ssdt(const struct device *dev)
{
	const struct device *igd = pcidev_path_on_root(PCI_DEVFN(2, 0));
	const igd_opregion_t *opregion = cbmem_find(CBMEM_ID_IGD_OPREGION);
	const uintptr_t base = (uintptr_t)opregion;

	if (!nec_igd_opregion_valid(igd, opregion))
		return;

	printk(BIOS_INFO, "NEC IGD before payload: CMD=%04x SSID=%08x MSAC=%02x\n",
	       pci_read_config16(igd, PCI_COMMAND), pci_read_config32(igd, PCI_SUBSYSTEM_VENDOR_ID),
	       pci_read_config8(igd, 0x62));

	const struct opregion graphics = OPREGION("NORM", SYSTEMMEMORY, base,
						 sizeof(*opregion));
	const struct opregion tco = OPREGION("NTCO", SYSTEMIO, tco_get_bar() + TCO1_STS, 2);

	acpigen_write_scope("\\_SB.PCI0.GFX0");
	nec_igd_write_nvs();
	acpigen_write_opregion(&graphics);
	acpigen_write_field("NORM", graphics_fields, ARRAY_SIZE(graphics_fields),
			    FIELD_DWORDACC | FIELD_NOLOCK | FIELD_PRESERVE);
	acpigen_write_opregion(&tco);
	/* TCO1_STS is W1C. Only write the CPU SCI status bit. */
	acpigen_write_field("NTCO", tco_fields, ARRAY_SIZE(tco_fields),
			    FIELD_WORDACC | FIELD_NOLOCK | FIELD_WRITEASZEROS);
	acpigen_write_name_dword("NORB", base);
	acpigen_pop_len();
}

static struct device_operations nec_root_ops;
static void (*original_fill_ssdt)(const struct device *dev);

static void nec_root_fill_ssdt(const struct device *dev)
{
	if (original_fill_ssdt)
		original_fill_ssdt(dev);

	nec_igd_fill_ssdt(dev);
}

static void nec_igd_enable(struct device *dev)
{
	if (dev->ops == &nec_root_ops)
		return;

	nec_root_ops = *dev->ops;
	original_fill_ssdt = nec_root_ops.acpi_fill_ssdt;
	nec_root_ops.acpi_fill_ssdt = nec_root_fill_ssdt;
	dev->ops = &nec_root_ops;
}

struct chip_operations mainboard_ops = {
	.enable_dev = nec_igd_enable,
};
