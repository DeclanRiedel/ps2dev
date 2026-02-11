{
  description = "PS2 Development Environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        # PS2Dev profile script
        ps2devProfile = pkgs.writeText "ps2dev-profile" ''
          export PS2DEV=$HOME/.ps2dev
          export PS2SDK=$PS2DEV/ps2sdk
          export GSKIT=$PS2DEV/gsKit
          export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin

          setup_ps2dev_toolchain() {
            if [ ! -d "$PS2DEV/ee" ]; then
              echo "Setting up PS2Dev toolchain on first use..."
              mkdir -p "$PS2DEV"
              cd "$PS2DEV"
              curl -o ps2dev-latest.tar.gz -LC - https://github.com/ps2dev/ps2dev/releases/download/latest/ps2dev-ubuntu-latest.tar.gz
              tar -xf ps2dev-latest.tar.gz --strip-components 1
              rm ps2dev-latest.tar.gz
              echo "PS2Dev toolchain installed!"
              echo ""
            fi
          }

          # Auto-setup on shell entry
          setup_ps2dev_toolchain

          echo ""
          echo "╔══════════════════════════════════════════════════════════════╗"
          echo "║           PS2 Development Environment                         ║"
          echo "╚══════════════════════════════════════════════════════════════╝"
          echo ""
          echo "Toolchain Location: $PS2DEV"
          echo ""

          # Quick verification
          if [ -d "$PS2DEV/ee" ] && [ -f "$PS2DEV/ee/bin/mips64r5900el-ps2-elf-gcc" ]; then
            echo "Toolchain: VERIFIED"
          else
            echo "⚠️  Toolchain: NOT FOUND"
          fi

          echo ""
          echo "╔══════════════════════════════════════════════════════════════╗"
          echo "║                    Available Commands                        ║"
          echo "╚══════════════════════════════════════════════════════════════╝"
          echo ""
          echo "  nix run .#clean              Clean build artifacts"
          echo "  nix run .#clean-all          Clean all (current/, src/, bin/)"
          echo "  nix run .#build-elf -- NAME  Build NAME.elf → bin/ + src/"
          echo "  nix run .#build-iso -- NAME  Build NAME.iso → bin/"
          echo "  nix run .#test-controller    Test USB controller input"
          echo ""
          echo "═══════════════════════════════════════════════════════════════"
          echo ""
         '';

          # PS2Dev environment using FHS for prebuilt binaries
           ps2devEnv = pkgs.buildFHSEnv {
             name = "ps2dev-env";
             targetPkgs = pkgs: [
               pkgs.bash
               pkgs.coreutils
               pkgs.gzip
               pkgs.gnutar
               pkgs.curl
               pkgs.git
               pkgs.gnumake
               pkgs.cmake
               pkgs.gcc
               pkgs.cdrtools
               pkgs.evtest
               pkgs.joystickwake
               pkgs.linuxConsoleTools
               pkgs.sdl2-compat
               pkgs.python3
             ];
             profile = ps2devProfile;
             runScript = "bash";
           };

          # PCSX2 build environment
           pcsx2Env = pkgs.mkShell {
             buildInputs = with pkgs;
               with qt6;
               with xorg; [
                 curl
                 extra-cmake-modules
                 ffmpeg
                 libaio
                 libbacktrace
                 libpcap
                 libwebp
                 libXrandr
                 lz4
                 qtbase
                 qtsvg
                 qttools
                 qtwayland
                 SDL2
                 shaderc
                 soundtouch
                 vulkan-headers
                 wayland
                 zstd
                 clang
                 lld
               ];

             nativeBuildInputs = with pkgs;
               with qt6; [
                 cmake
                 pkg-config
                 strip-nondeterminism
                 wrapQtAppsHook
                 zip
               ];

             qtWrapperArgs = let
               libs = with pkgs;
                 lib.makeLibraryPath
                   ([ vulkan-loader shaderc ] ++ cubeb.passthru.backendLibs);
             in [ "--prefix LD_LIBRARY_PATH : ${libs}" ];

             shellHook = ''
               if [ ! -d "$(pwd)/pcsx2" ]; then
                 echo "pcsx2 not cloned, cloning..."
                 git clone git@github.com:PCSX2/pcsx2.git
               fi
               if [ ! -d "$(pwd)/pcsx2/build" ]; then
                 echo "pcsx2 not configured, configuring..."
                 cd pcsx2
                 cmake -B build -DDISABLE_ADVANCE_SIMD=true -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXE_LINKER_FLAGS_INIT="-fuse-ld=lld" -DCMAKE_MODULE_LINKER_FLAGS_INIT="-fuse-ld" -DCMAKE_SHARED_LINKER_FLAGS_INIT="-fuse-ld=lld" -DCMAKE_PREFIX_PATH="$PWD/deps" -GNinja
                 cd ..
               fi

               bashdir=$(mktemp -d)
               makeWrapper "$(type -p bash)" "$bashdir/bash" "''${qtWrapperArgs[@]}"
               export NIX_ENFORCE_PURITY=0
               exec "$bashdir/bash"
             '';
           };

        # Build script that uses make
        buildScript = pkgs.writeShellScriptBin "build-ps2" ''
          set -e
          TARGET="''${1:-demo}"

          # Set up environment
          export PS2DEV=$HOME/.ps2dev
          export PS2SDK=$PS2DEV/ps2sdk
          export GSKIT=$PS2DEV/gsKit
          export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin

          # Use make to build
          make TARGET="$TARGET" all
        '';

        # Run script
        runScript = pkgs.writeShellScriptBin "run-ps2" ''
          set -e
          TARGET="''${1:-demo}"

          # Set up environment
          export PS2DEV=$HOME/.ps2dev
          export PS2SDK=$PS2DEV/ps2sdk
          export GSKIT=$PS2DEV/gsKit
          export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin

          # Build first
          make TARGET="$TARGET" all

          # Try to run in PCSX2
          if command -v pcsx2-emulator &> /dev/null; then
            echo "Built ELF: $TARGET.elf"
            echo "Launch PCSX2 and load: $(pwd)/$TARGET.elf"
            pcsx2-emulator
          elif command -v pcsx2-qt &> /dev/null; then
            pcsx2-qt "$(pwd)/$TARGET.elf"
          elif command -v pcsx2 &> /dev/null; then
            pcsx2 "$(pwd)/$TARGET.elf"
          elif command -v PCSX2 &> /dev/null; then
            PCSX2 "$(pwd)/$TARGET.elf"
          else
            echo "PCSX2 not found in PATH."
            echo "Built ELF: $TARGET.elf"
            echo "Load this file manually in PCSX2"
          fi
        '';

         # Clean script
         cleanScript = pkgs.writeShellScriptBin "clean-ps2" ''
           make clean
         '';

         # Clean-all script
          cleanAllScript = pkgs.writeShellScriptBin "clean-all-ps2" ''
            make clean
            rm -rf bin/* src/* 2>/dev/null || true
            echo "✓ Cleaned all directories (current/, src/, bin/)"
          '';

          # Test controller script
          testControllerScript = pkgs.writeShellScriptBin "test-controller" ''
            set -e
            echo "=========================================="
            echo "Controller Input Test"
            echo "=========================================="
            echo ""

            # Add linuxconsoletools to PATH
            export PATH="${pkgs.linuxConsoleTools}/bin:$PATH"

            # Check if controller exists
            if [ ! -e /dev/input/js0 ]; then
              echo "✗ No controller found at /dev/input/js0"
              echo ""
              echo "Make sure your controller is plugged in."
              exit 1
            fi

            echo "✓ Controller found: $(ls -la /dev/input/js0 | awk '{print $NF}')"
            echo ""

            # Check permissions
            if [ -r /dev/input/js0 ]; then
              echo "✓ Can read from controller"
            else
              echo "✗ Permission denied reading controller"
              echo ""
              echo "Fix with: sudo usermod -a -G input \$USER"
              echo "Then logout and login again"
              exit 1
            fi

            echo ""
            echo "Testing controller input - press buttons! (Ctrl+C to exit)"
            echo ""

            # Run jstest
            exec jstest /dev/input/js0
          '';

          # Build ELF script
         buildElfScript = pkgs.writeShellScriptBin "build-elf-ps2" ''
           set -e
           NAME="''${1:-demo}"

           # Set up environment
           export PS2DEV=$HOME/.ps2dev
           export PS2SDK=$PS2DEV/ps2sdk
           export GSKIT=$PS2DEV/gsKit
           export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin

           # Ensure directories exist
           mkdir -p src bin

           echo "=========================================="
           echo "  Building PS2 ELF: $NAME"
           echo "=========================================="
           echo ""

           # Determine source file
           if [ -f "main.c" ]; then
             SOURCE_FILE="main.c"
           elif [ -f "src/$NAME.c" ]; then
             SOURCE_FILE="src/$NAME.c"
           elif [ -f "$NAME.c" ]; then
             SOURCE_FILE="$NAME.c"
           else
             echo "Error: No source file found!"
             echo "Looking for: main.c, src/$NAME.c, or $NAME.c"
             exit 1
           fi

           echo "Source: $SOURCE_FILE"
           echo "Building: bin/$NAME.elf"
           echo ""

           # Copy source to src/ if it's not already there
           if [ "$SOURCE_FILE" != "src/$NAME.c" ]; then
             cp "$SOURCE_FILE" "src/$NAME.c"
             echo "✓ Copied source to src/$NAME.c"
           fi

           # Compile directly
           PS2DEV=$HOME/.ps2dev
           PS2SDK=$PS2DEV/ps2sdk
           GSKIT=$PS2DEV/gsKit

           EE_CC=mips64r5900el-ps2-elf-gcc
           EE_CFLAGS="-D_EE -O2 -G0 -Wall"
           EE_CFLAGS="$EE_CFLAGS -I$PS2SDK/ee/include"
           EE_CFLAGS="$EE_CFLAGS -I$PS2SDK/common/include"
           EE_CFLAGS="$EE_CFLAGS -I$PS2SDK/sbv/include"
           EE_CFLAGS="$EE_CFLAGS -I$GSKIT/include"
           EE_CFLAGS="$EE_CFLAGS -I$GSKIT/ee/gs/include"
           EE_CFLAGS="$EE_CFLAGS -I$GSKIT/ee/include"

           EE_LDFLAGS="-L$PS2SDK/ee/lib"
           EE_LDFLAGS="$EE_LDFLAGS -L$PS2DEV/ee/lib"
           EE_LDFLAGS="$EE_LDFLAGS -L$GSKIT/lib"
           EE_LDFLAGS="$EE_LDFLAGS -L$PS2SDK/ee/lib/startup"
           EE_LDFLAGS="$EE_LDFLAGS -T$PS2SDK/ee/startup/linkfile"

            EE_LIBS="-lkernel -lcglue -lcdvd -lgskit -ldmakit -lpacket -lpad -lm"

           echo "Compiling..."
           $EE_CC $EE_CFLAGS -c "src/$NAME.c" -o "$NAME.o"

           # Copy object file to src/
           cp "$NAME.o" "src/$NAME.o"
           echo "✓ Copied object file to src/$NAME.o"

           echo "Linking..."
           $EE_CC $EE_LDFLAGS -o "bin/$NAME.elf" "$NAME.o" $EE_LIBS

           # Clean up temporary object file
           rm -f "$NAME.o"

           if [ -f "bin/$NAME.elf" ]; then
             echo ""
             echo "Build successful!"
             echo "Output: bin/$NAME.elf"
             ls -lh "bin/$NAME.elf" | awk '{print "  Size: " $5}'
             echo ""
           else
             echo "Build failed!"
             exit 1
           fi
         '';

         # Build ISO script
         buildIsoScript = pkgs.writeShellScriptBin "build-iso-ps2" ''
           set -e
           NAME="''${1:-demo}"

           export PS2DEV=$HOME/.ps2dev
           export PS2SDK=$PS2DEV/ps2sdk
           export GSKIT=$PS2DEV/gsKit
           export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin

           echo "=========================================="
           echo "  Building PS2 ISO: $NAME"
           echo "=========================================="
           echo ""

           # Check if ELF exists
           if [ ! -f "bin/$NAME.elf" ]; then
             echo "Error: bin/$NAME.elf not found!"
             echo ""
             echo "Build the ELF first:"
             echo "nix run .#build -- $NAME"
             exit 1
           fi

           echo "Creating: bin/$NAME.iso"
           echo ""

           # Note: mkisofs may not be available - inform user
           echo "⚠️  ISO creation requires cdrtools package"
           echo "ELF is ready at: bin/$NAME.elf"
           echo "You can load the ELF directly in PCSX2 instead."
         '';

      in {
        devShells.default = ps2devEnv;
        devShells.pcsx2 = pcsx2Env;

          packages = {
            default = self.packages.${system}.build-elf;
            build-elf = buildElfScript;
            build-iso = buildIsoScript;
            build = buildScript;
            run = runScript;
            clean = cleanScript;
            clean-all = cleanAllScript;
            test-controller = testControllerScript;
          };
          apps = {
            build-elf = {
              type = "app";
              program = "${buildElfScript}/bin/build-elf-ps2";
            };
            build-iso = {
              type = "app";
              program = "${buildIsoScript}/bin/build-iso-ps2";
            };
            build = {
              type = "app";
              program = "${buildScript}/bin/build-ps2";
            };
            run = {
              type = "app";
              program = "${runScript}/bin/run-ps2";
            };
            clean = {
              type = "app";
              program = "${cleanScript}/bin/clean-ps2";
            };
            "clean-all" = {
              type = "app";
              program = "${cleanAllScript}/bin/clean-all-ps2";
            };
            "test-controller" = {
              type = "app";
              program = "${testControllerScript}/bin/test-controller";
            };
          };
      }
    );
}
