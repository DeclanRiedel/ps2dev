#!/usr/bin/env python3
"""
Test USB controller input on Linux using SDL2
Run with: nix develop --command python3 test_controller.py
"""

import sys
import time

try:
    import pygame
    pygame.init()
except ImportError:
    print("Error: pygame not found")
    print("Install: nix-shell -p python3 python3Packages.pygame --run 'python3 test_controller.py'")
    sys.exit(1)

print("=" * 60)
print("USB Controller Input Test")
print("=" * 60)
print()

# Initialize joystick
joystick_count = pygame.joystick.get_count()
print(f"Found {joystick_count} controller(s)")

if joystick_count == 0:
    print("\nNo controllers detected!")
    print("\nTroubleshooting:")
    print("1. Check controller is connected:")
    print("   ls -la /dev/input/js*")
    print("\n2. Test with jstest:")
    print("   nix develop --command 'jstest /dev/input/js0'")
    print("\n3. Check permissions:")
    print("   groups $USER | grep input")
    print("   # If not in input group:")
    print("   sudo usermod -a -G input $USER")
    print("   # Then logout and login again")
    sys.exit(1)

# Use first controller
joystick = pygame.joystick.Joystick(0)
joystick.init()

print(f"\nController: {joystick.get_name()}")
print(f"Axes: {joystick.get_numaxes()}")
print(f"Buttons: {joystick.get_numbuttons()}")
print(f"Hats: {joystick.get_numhats()}")
print()

print("=" * 60)
print("Press buttons and move sticks... (Ctrl+C to exit)")
print("=" * 60)
print()

try:
    while True:
        for event in pygame.event.get():
            if event.type == pygame.JOYBUTTONDOWN:
                print(f"Button {event.button} PRESSED")
            elif event.type == pygame.JOYBUTTONUP:
                print(f"Button {event.button} released")
            elif event.type == pygame.JOYAXISMOTION:
                # Only print significant movement
                value = round(event.value, 2)
                if abs(value) > 0.1:
                    print(f"Axis {event.axis}: {value}")
            elif event.type == pygame.JOYHATMOTION:
                print(f"Hat {event.hat}: {event.value}")

        time.sleep(0.01)

except KeyboardInterrupt:
    print("\n\nExiting...")
    pygame.quit()
