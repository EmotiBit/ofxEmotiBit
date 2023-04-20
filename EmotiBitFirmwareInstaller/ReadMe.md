# EmotiBit Firmware Installer
ToDo: Add details about implementation here.


# Tools used to install firmware

## Feather M0
- bossac
  - todo: add details about bossac
## Feather ESP32

- esptool
  - EmotiBit Firmware Installer is currently using [`esptool v3.3`](https://github.com/espressif/esptool/releases/tag/v3.3) 
to flash firmware binaries on ESP32 Feather
  - Note for macOS
    - After the esptool binary is obtained from the release page, it is made an executable by using the `chmod u+x` command.