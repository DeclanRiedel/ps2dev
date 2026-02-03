# PS2 Dev flake

# Enter the development environment (verifies toolchain and shows commands)
nix develop

## Available Commands
```bash
# Clean build artifacts in current directory
nix run .#clean
# Clean all build artifacts (current/, src/, bin/)
nix run .#clean-all
# Build an ELF from main.c (or src/NAME.c)
nix run .#build-elf -- NAME
# Build an ISO from NAME.elf
nix run .#build-iso -- NAME
```
# Load in PCSX2: File → Boot ELF → bin/main.elf
## pcsx2-emulator path/to/file.elf (note: idk sometimes it runs .elf as i think .iso and breaks?)

### Traditional Make Commands

```bash
# Using the Makefile directly
make all
make clean

# Run PCSX2 (opens GUI - load ELF manually)
make run
```
## Toolchain Verification
-  Checks if PS2Dev toolchain is installed at `$HOME/ps2dev`
-  Verifies critical components (EE compiler, PS2SDK, gsKit)
-  Shows all available commands with descriptions
-  Provides installation instructions if toolchain is missing

### Installing the Toolchain (if missing)
#### See: https://github.com/ps2dev/ps2dev

```bash
mkdir -p $HOME/ps2dev
cd $HOME/ps2dev
curl -o ps2dev-latest.tar.gz -LC - https://github.com/ps2dev/ps2dev/releases/download/latest/ps2dev-ubuntu-latest.tar.gz
tar -xf ps2dev-latest.tar.gz --strip-components 1
rm ps2dev-latest.tar.gz
```
