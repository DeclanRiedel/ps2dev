#!/bin/bash
# Test controller permissions and setup

echo "=========================================="
echo "Controller Input Test & Setup"
echo "=========================================="
echo ""

# Check if controller is connected
echo "1. Checking for controllers..."
if ls /dev/input/js* >/dev/null 2>&1; then
    echo "   ✓ Found controller(s):"
    ls -la /dev/input/js*
else
    echo "   ✗ No controllers found at /dev/input/js*"
    echo ""
    echo "   Troubleshooting:"
    echo "   - Make sure controller is plugged in"
    echo "   - Try different USB port"
    echo "   - Check dmesg for errors: dmesg | tail -20"
    exit 1
fi

echo ""

# Check permissions
echo "2. Checking permissions..."
DEV="/dev/input/js0"
if [ -e "$DEV" ]; then
    PERMS=$(ls -la "$DEV" | awk '{print $1, $3, $4}')
    echo "   Permissions: $PERMS"

    if groups $USER | grep -q input; then
        echo "   ✓ User '$USER' is in 'input' group"
    else
        echo "   ✗ User '$USER' is NOT in 'input' group"
        echo ""
        echo "   Fix: sudo usermod -a -G input \$USER"
        echo "   Then log out and back in"
        echo ""
        echo "   Quick fix (current session only):"
        echo "   sudo chmod 666 $DEV"
    fi
fi

echo ""

# Test controller with jstest
echo "3. Testing controller with jstest..."
echo "   (Press buttons to see input, Ctrl+C to exit)"
echo ""
if command -v jstest >/dev/null 2>&1; then
    timeout 5 jstest /dev/input/js0 || true
    echo ""
    echo "   If you saw button presses above, controller works!"
else
    echo "   ✗ jstest not found"
    echo "   Run: nix develop (this should provide jstest)"
fi

echo ""

# List all input devices
echo "4. All input devices:"
ls -la /dev/input/ | grep -E "js|event"

echo ""
echo "=========================================="
echo "Testing complete!"
echo "=========================================="
