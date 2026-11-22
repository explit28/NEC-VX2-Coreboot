# Test matrix

| Test | Result | Evidence / notes |
|---|---|---|
| Coreboot boot stages reach EDK2 payload | PASS | Persistent SPI console: `Jumping to boot code` |
| libgfxinit eDP AUX / external DDC probing | FAIL | EDID reads failed; no display selected |
| Legacy Intel VGA ROM launch | PASS | `Option ROM returned`, `VGA Option ROM was run` |
| Legacy VESA 1024x768 visible output | FAIL | Backlight on, black screen |
| Legacy VESA 1280x1024 visible output | FAIL | Backlight on, black screen |
| OEM Intel GOP + VBT graphical output | **PASS** | Visible coreboot logo in EDK2 |
| EDK2 UEFI Shell | **PASS** | Shell appeared after splash |
| EDK2 graphical setup (LVGL + NEC OEM GOP) | **PASS** | Graphical Firmware Setup photographed on the test board |
| USB boot menu entries | FAIL | USB boot devices did not appear in the latest test. USB enumeration and boot media compatibility are still unverified. |
| External USB keyboard in EDK2 | NOT TESTED | Pending |
| Internal SSD detection | NOT TESTED | No SSD installed at first successful test |
| External HDMI/VGA video with OEM GOP | NOT TESTED | No independently confirmed result |
| OS boot, suspend, audio, keyboard, TPM, networking | NOT TESTED | Pending |
