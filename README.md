# PS2 Development Environment

PlayStation 2 development environment with Nix flakes, PS2Dev toolchain, and controller support.
https://ps2dev.github.io/ps2sdk/index.html

## Quick Start

**First time setup - run once to download the PS2 toolchain (~250MB):**
```bash
nix develop
# Toolchain will auto-download to $HOME/.ps2dev
# Exit shell when done: exit
```

**Build your PS2 project:**
```bash
nix run .#build-elf -- pad
# Output: bin/pad.elf
```

**Available commands:**
```bash
nix run .#clean              Clean build artifacts
nix run .#clean-all          Clean all (current/, src/, bin/)
nix run .#build-elf -- NAME  Build NAME.elf from main.c
nix run .#build-iso -- NAME  Build NAME.iso from NAME.elf
nix run .#run -- NAME        Build and run in PCSX2
nix run .#test-controller    Test USB controller input
```

## Project Structure

```
.
├── main.c              # Your PS2 source code
├── Makefile            # Makefile for building
├── flake.nix           # Nix flake configuration
├── README.md           # This file
├── docs/
│   └── CONTROLLER_SETUP.md  # USB controller setup guide
 ├── tools/
 │   ├── controller_test.c        # Linux controller test program
 │   ├── test_controller.sh       # Controller diagnostic script
 │   ├── test_buttons.sh          # Quick button test
 │   ├── test_controller.py       # SDL2-based controller test
 │   └── setup_controller_ubuntu.sh  # Global Ubuntu setup script
├── src/                 # Copied source files (auto-generated)
├── bin/                 # Compiled ELF files
└── external/            # External dependencies
```

## Running in PCSX2

```bash
# Build and run
nix run .#run -- pad

# Or load ELF manually in PCSX2 GUI
# File → Boot ELF → select bin/pad.elf
```

## USB Controller Support

### Option 1: Nix Environment (Recommended)
Test your controller directly:
```bash
nix run .#test-controller
```

### Option 2: Global Ubuntu Setup
Install controller support system-wide:
```bash
sudo bash tools/setup_controller_ubuntu.sh
```

This installs:
- `evtest` - Event testing
- `joystick` / `jstest` - Controller testing
- `linuxconsoletools` - Input utilities
- `sdl2-compat` - SDL2 library

See [docs/CONTROLLER_SETUP.md](docs/CONTROLLER_SETUP.md) for detailed guide.

## Toolchain Location

The PS2Dev toolchain is installed at:
- **$HOME/.ps2dev/** - Toolchain directory (auto-downloaded)

## Manual Toolchain Installation (if auto-setup fails)

```bash
mkdir -p $HOME/.ps2dev
cd $HOME/.ps2dev
curl -o ps2dev-latest.tar.gz -LC - https://github.com/ps2dev/ps2dev/releases/download/latest/ps2dev-latest.tar.gz
tar -xf ps2dev-latest.tar.gz --strip-components 1
rm ps2dev-latest.tar.gz
```

**Build your project:**
```bash
nix run .#build-elf -- exam
# Output: bin/exam.elf
```

**Available commands:**
```bash
nix run .#clean         Clean build artifacts
nix run .#clean-all     Clean all (current/, src/, bin/)
nix run .#build-elf -- NAME  Build NAME.elf from main.c
nix run .#build-iso -- NAME  Build NAME.iso from NAME.elf
```

## Running in PCSX2

```bash
pcsx2-emulator bin/exam.elf
```

Or via GUI: File → Boot ELF → select `bin/exam.elf`

## Toolchain Location

The PS2Dev toolchain is installed at:
- **$HOME/.ps2dev/** - Toolchain directory (auto-downloaded)

## Manual Toolchain Installation (if auto-setup fails)

```bash
mkdir -p $HOME/.ps2dev
cd $HOME/.ps2dev
curl -o ps2dev-latest.tar.gz -LC - https://github.com/ps2dev/ps2dev/releases/download/latest/ps2dev-ubuntu-latest.tar.gz
tar -xf ps2dev-latest.tar.gz --strip-components 1
rm ps2dev-latest.tar.gz
```

## pickup test controller from ubuntu idk
sudo snap connect pcsx2-emulator:joystick
