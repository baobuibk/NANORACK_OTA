# Firmware Update Package Creation (FOTA)

## Requirements
- The firmware **must be built for the bootloader**.
- Python 3.x installed.
- Recommended directory layout:
  ```text
  ./
  ├─ Firmware/     # Place input .bin files here
  ├─ Output/       # Generated .bin + .json pairs will appear here
  ├─ sha256_gen.py
  ├─ sha256_verify.py  
  └─ FOTA.py
### Step 1: Prepare the .bin firmware
 - The file name must include the board name, e.g., EXP/exp or OBC/obc.
 - Copy the firmware .bin file into ./Firmware.
> Note: Ensure this firmware build targets the bootloader.

### Step 2: Generate metadata and release bundle
 - Run the metadata/checksum generator:
```
python sha256_gen.py
```
 - The script will generate a pair of files for each firmware: the .bin and a matching .json in ./Output.
 - Zip/warp each pair (.bin + .json) to send to the Adapter for flashing via the bootloader.

### Step 3: Flash the firmware via Adapter (FOTA)
 - On the flashing machine, run:
```
python FOTA.py
```
 - Then follow this order:
  1. Reset the device into bootloader mode.
  2. Check the connection (for EXP boards, wait at least 2 seconds before selecting the device).
  4. Choose the .bin file that matches the target board - Flash the firmware. <br /> -> Wait for completion and confirm success.

