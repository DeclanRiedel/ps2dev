# PS2 Dev flake

## Quick Start

**First time setup - run once to download the PS2 toolchain (~250MB):**
```bash
nix develop
# Toolchain will auto-download to $HOME/.ps2dev
# Exit shell when done: exit
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
