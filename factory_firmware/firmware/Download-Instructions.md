# RotaryScreen 1.46 Firmware Download Guide

> Version date: 2026-09-16
> What's new in this release: improved touch swipe feel when selecting items in the main menu (touch response on the detail page improved as well)

## Firmware Configuration

- Chip: ESP32-S3 (requires 16 MB Flash + OPI PSRAM)
- Arduino ESP32 Core: 3.3.8-cn
- Flash: 16 MB, 80 MHz, DIO
- PSRAM: OPI PSRAM
- Partition scheme: elecrow_s3 (10 MB APP)
- Boot sequence: shows the Screen0 splash image, enters the main menu (Screen1) after about 1.5 seconds

## Interaction Notes for This Version

| Action | Location | Effect |
| --- | --- | --- |
| Swipe left/right (≥30 px) | Main menu | Switch between Volume / Temp / Light, one step per swipe |
| Rotate the knob | Main menu | Same as above, switches one step at a time |
| Short press the knob | Main menu | Enter the detail page of the currently selected item |
| Drag the ring | Detail page | Adjust volume / temperature / brightness |
| Double press the knob | Detail page | Return to the main menu |

Main menu swiping uses a **one step per touch** strategy: continuing to swipe without lifting your finger will not skip through multiple steps — you need to lift your finger and swipe again to advance another step. This is an intentional design choice to avoid accidentally scrolling past several options on a mis-touch.

## Recommended Method: Flash the Complete Firmware in One Go

File to use: `RotaryScreen_1_46_ESP32S3_16MB_merged.bin` (16 MB, contains bootloader + partition table + application)

1. Connect the board with a USB cable.
2. Check Device Manager for the newly added COM port, for example `COM6`.
3. If the device does not enter download mode automatically: hold BOOT, briefly press RESET, then release BOOT.
4. Open a terminal in this directory and run:

```bat
flash_firmware.bat COM6
```

Replace `COM6` with the actual port. The device resets automatically after flashing succeeds.

You can also run esptool directly:

```bat
esptool.exe --chip esp32s3 --port COM6 --baud 921600 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 RotaryScreen_1_46_ESP32S3_16MB_merged.bin
```

If the connection is unstable, change the baud rate from `921600` to `115200`.

## Espressif Flash Download Tool

1. Select `ESP32-S3` as the chip and `Develop` as the mode.
2. Add `RotaryScreen_1_46_ESP32S3_16MB_merged.bin`.
3. Set the download address to `0x0`.
4. Set SPI SPEED to `80MHz`, SPI MODE to `DIO`, and FLASH SIZE to `16MB`.
5. Select the correct COM port and click START.

## Separate Firmware Flash Addresses

If you need to download the files separately, the files and addresses are as follows:

| Address | File |
| --- | --- |
| `0x0000` | `bootloader.bin` |
| `0x8000` | `partitions.bin` |
| `0xE000` | `boot_app0.bin` |
| `0x10000` | `RotaryScreen_1_46_app.bin` |

Note: when updating only the application, flash just `RotaryScreen_1_46_app.bin` to `0x10000`; for a first-time flash or when the partition layout changes, use the complete merged firmware.

## SHA-256 Checksums

See `SHA256SUMS.txt` in the same directory for the full list.

```text
RotaryScreen_1_46_ESP32S3_16MB_merged.bin  a19d0791915d7bf4e098c4388c402b202f067aa18c66ec787de13995972835cd
RotaryScreen_1_46_app.bin                  7330dbcaa128e9011c08a478efa70d9e7593d83b7891748ba2d3a0be162009bc
bootloader.bin                             b68c1ed33c43a4290375213f24d608c125e31e22880d16d4a5dcba89e083716d
partitions.bin                             0b9c3ff6810d66db6f419056f0fe49b118d8cf251049677e9475fae3fd0fb5d3
boot_app0.bin                              f94c5d786a7a8fab06ac5d10e33bf37711a6697636dc037559ea19cc410a17f0
```

Verification command on Windows:

```bat
certutil -hashfile RotaryScreen_1_46_ESP32S3_16MB_merged.bin SHA256
```

## FAQ

- **Stuck at `Connecting...`**: manually enter download mode (hold BOOT → briefly press RESET → release BOOT) and try again.
- **COM port not found**: use a USB cable that supports data transfer, or install the appropriate USB-to-serial driver.
- **Fails mid-flash**: close the serial monitor and lower the baud rate to `115200`.
- **Black screen or reset loop after flashing**: make sure the target hardware is an ESP32-S3 variant with 16 MB Flash and OPI PSRAM; a mismatched Flash or PSRAM configuration will cause boot failure.
- **Touch swipe still not responsive**: make sure you flashed the firmware from this directory (2026-09-16 version); older firmware used a different swipe threshold and trigger strategy in the main menu.

## File List

| File | Description |
| --- | --- |
| `RotaryScreen_1_46_ESP32S3_16MB_merged.bin` | Complete merged firmware, recommended |
| `RotaryScreen_1_46_app.bin` | Application only, for incremental updates on boards that have already been flashed with the complete firmware |
| `bootloader.bin` | Second-stage bootloader, address `0x0000` |
| `partitions.bin` | Partition table, address `0x8000` |
| `boot_app0.bin` | OTA boot data, address `0xE000` |
| `flash_firmware.bat` | One-click flashing script |
| `SHA256SUMS.txt` | SHA-256 checksum list |
