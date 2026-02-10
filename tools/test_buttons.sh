#!/bin/bash
# Quick controller button test - reads 5 seconds of input

echo "Testing controller - press buttons now! (5 seconds)"
echo ""

if [ -r /dev/input/js0 ]; then
    # Use hexdump to show raw input data
    timeout 5 hexdump -C /dev/input/js0 2>/dev/null | head -20 || true
    echo ""
    echo "✓ Controller is readable!"
    echo "✓ If you pressed buttons and saw hex data above, input is working!"
else
    echo "✗ Cannot read /dev/input/js0"
    echo ""
    echo "Fix permissions with:"
    echo "  sudo usermod -a -G input \$USER"
    echo "  (then logout and login again)"
    echo ""
    echo "Or temporarily:"
    echo "  sudo chmod 666 /dev/input/js0"
fi
