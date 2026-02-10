#!/bin/bash
# setup_controller_ubuntu.sh - Install and configure USB controller support on Ubuntu
# Run with: sudo bash tools/setup_controller_ubuntu.sh

set -e

echo "=========================================="
echo "USB Controller Setup for Ubuntu"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "This script requires root privileges for package installation."
    echo "Please run with: sudo bash $0"
    exit 1
fi

# Get the non-root username
USERNAME=$SUDO_USER
if [ -z "$USERNAME" ]; then
    echo "Error: Could not determine username"
    exit 1
fi

echo "Installing controller support packages..."
echo ""

# Update package list
apt-get update

# Install necessary packages
# Note: Package names vary by Ubuntu version
apt-get install -y \
    evtest \
    joystick \
    inputattach \
    jstest-gtk \
    libsdl2-2.0-0 \
    python3-pygame \
    || apt-get install -y \
    evtest \
    joystick \
    inputattach \
    python3-pygame

# Try to install jstest if available (may be in different packages)
if ! command -v jstest &> /dev/null; then
    echo ""
    echo "Note: jstest not found in standard repositories."
    echo "You can compile it from source or use the Nix environment instead:"
    echo "  nix run .#test-controller"
fi

echo ""
echo "✓ Core packages installed successfully"
echo ""

# Check if jstest is available
if command -v jstest &> /dev/null; then
    echo "✓ jstest is available at: $(which jstest)"
else
    echo "⚠️  jstest not found - testing with alternative methods"
    echo ""
    echo "To get jstest, you can:"
    echo "  1. Use Nix environment (recommended): nix run .#test-controller"
    echo "  2. Compile from source: https://github.com/Grumbel/jstest"
    echo "  3. Use evtest instead: evtest /dev/input/js0"
fi

echo ""

# Add user to input group
echo "Adding user '$USERNAME' to 'input' group..."
usermod -a -G input "$USERNAME"
echo "✓ User added to input group"
echo ""

# Check if controller is connected
echo "Checking for connected controllers..."
if ls /dev/input/js* >/dev/null 2>&1; then
    echo "✓ Found controller(s):"
    ls -la /dev/input/js*
    echo ""

    # Set permissions for current session
    echo "Setting permissions for current session..."
    chmod 666 /dev/input/js* 2>/dev/null || true
    echo "✓ Permissions set"
else
    echo "⚠️  No controllers found at /dev/input/js*"
    echo "   Please connect your controller and run this script again"
fi

echo ""
echo "=========================================="
echo "Setup Complete!"
echo "=========================================="
echo ""
echo "IMPORTANT: Log out and log back in for group changes to take effect"
echo ""

# Check what tools are available
echo "Available testing methods:"
if command -v jstest &> /dev/null; then
    echo "  - jstest /dev/input/js0        (Command-line test)"
fi
if command -v evtest &> /dev/null; then
    echo "  - evtest /dev/input/js0        (Detailed event test)"
fi
if command -v jstest-gtk &> /dev/null; then
    echo "  - jstest-gtk                  (GUI test)"
fi
echo ""
echo "Project test tools:"
echo "  - bash tools/test_buttons.sh           (Quick test)"
echo "  - gcc -o tools/controller_test tools/controller_test.c && ./tools/controller_test"
echo ""
echo "Nix environment (always works):"
echo "  - nix run .#test-controller"
echo ""
