# USB Controller Setup on Linux

## Quick Test

Test your controller immediately:
```bash
nix run .#test-controller
```

## Global Ubuntu Setup (System-wide)

If you want to use your controller with native Linux applications (outside of Nix):

```bash
sudo bash tools/setup_controller_ubuntu.sh
```

This installs controller support system-wide:
- `evtest` - Event testing
- `joystick` / `jstest` - Controller testing (when available)
- `inputattach` - Attach specialized controllers
- SDL2 library support
- Adds your user to the `input` group

**Note:** Package names vary by Ubuntu version. Some tools like `jstest` may not be available in all Ubuntu repositories. The script will install what's available and suggest alternatives.

**Important:** Log out and log back in after running this script for group changes to take effect.

**Alternative:** Use Nix environment for reliable controller testing:
```bash
nix run .#test-controller
```

## Troubleshooting Controller Issues

### 1. Check if controller is detected

```bash
ls -la /dev/input/js*
```

You should see something like:
```
crw-rw-r--+ 1 root input 13, 0 Feb  9 19:40 /dev/input/js0
```

### 2. Fix Permissions (if needed)

**Permanent fix:**
```bash
sudo usermod -a -G input $USER
# Then logout and login again
```

**Temporary fix (current session only):**
```bash
sudo chmod 666 /dev/input/js0
```

### 3. Test Controller Input

**Using jstest:**
```bash
nix develop
jstest /dev/input/js0
```

**Using the C test program:**
```bash
gcc -o tools/controller_test tools/controller_test.c
./tools/controller_test
```

**Using the quick test script:**
```bash
bash tools/test_buttons.sh
```

### 4. Common Controller Issues

#### Controller not detected
- Try different USB port
- Check `dmesg | tail -20` for kernel messages
- Some controllers need drivers (e.g., xboxdrv for Xbox controllers)

#### Permission denied
- User not in `input` group (see step 2)
- Device has wrong permissions

#### Controller detected but no input
- Controller might be in "sleep mode" - press a button to wake it
- Try `joystickwake` utility:
  ```bash
  nix develop
  joystickwake /dev/input/js0
  ```

#### Wrong button mapping
- Different controllers have different mappings
- Test with jstest to see your controller's actual button numbers
- Configure PCSX2 accordingly (see below)

## Using with PCSX2

1. **Open PCSX2** → `Settings` → `Controllers` → `Pad Settings`

2. **Map your controller:**
   - Click each button field
   - Press corresponding button on your controller
   - Test with `nix run .#run -- pad` after mapping

3. **If PCSX2 doesn't detect controller:**
   - Make sure controller works in Linux first (test with jstest)
   - Check PCSX2 settings for input backend (SDL/XInput)
   - Try running PCSX2 with: `SDL_JOYSTICK_DEVICE=/dev/input/js0 pcsx2-qt`

## Controller Mapping Reference

Your Redragon controller presents as "Microsoft X-Box 360 pad":
- **11 buttons:** A, B, X, Y, LB, RB, Select, Start, Mode, Left Stick, Right Stick
- **8 axes:** Left stick (X,Y), Right stick (X,Y), LT, RT, D-Pad (X,Y)

Typical PS2 mapping for Xbox-style controllers:
- **Cross (X)** = A (button 0)
- **Circle (O)** = B (button 1)
- **Square (□)** = X (button 2)
- **Triangle (△)** = Y (button 3)
- **L1** = LB (button 4)
- **R1** = RB (button 5)
- **L2** = Left Trigger (axis 2)
- **R2** = Right Trigger (axis 5)
- **D-Pad** = Hat axes (6, 7)

## Tools Available in Nix Environment

- `jstest` - Test controller input
- `evtest` - Detailed event testing
- `joystickwake` - Wake up sleeping controllers
- `SDL2` - For games/programs using SDL input

## Testing Your PS2 Program

After building your PS2 ELF:
```bash
nix run .#build-elf -- pad
nix run .#run -- pad
```

Press buttons in PCSX2 - your `pad.elf` will read input through the emulator!
