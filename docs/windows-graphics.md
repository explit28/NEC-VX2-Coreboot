# Windows graphics limitation

The owner reports that Windows boots on the NEC VersaPro VX-2 with the reference firmware, but the Intel UHD 620 driver still reports Code 43. This remains unresolved and was not analyzed again for the final hardware retest. The project does not claim that Windows graphics acceleration works.

The reference firmware uses the Intel OEM GOP and VBT through EDK2 for preboot display. A working boot menu, UEFI Shell, or framebuffer does not demonstrate successful Windows driver initialization or 3D acceleration. Earlier graphics experiments are preserved in the `archive-experimental-20261128` branch; this repository's Gerrit source snapshot does not include those experimental IGD, ACPI, DSM or SWSCI changes.
