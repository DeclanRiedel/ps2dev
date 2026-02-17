// alpha.c - Example demonstrating gsKit alpha blending operation.
// learn libdma -> DMA Chains
// libgif libc libkernel
// graphics -  libgs
// gskit -> 2d + 3d rendering help frame setup, texture upload, simple 3D,
// Vector Units libvu0?1 important to write custom VU microcode
// libvif Data transfer + geometery -> feeds geometry data into VU
// libpacket builds DMA + GIF packets cleanly
// audio: libspu2 audsrv
// input/o libpad libusb libmc

// #include <stdio.h>
// #include <malloc.h>
// #include <kernel.h>
// #include <stdio.h>
// #include <string.h>

// #include <tamtypes.h>
// #include <libpad.h>

// #include <libpad>
// #include <libkernel>

// #include <gsKit.h>
// #include <dmaKit.h>
// #include <gsToolkit.h>
// ###########################//

#include <kernel.h> //sleep, threads, vsync helpers
#include <sifrpc.h>
#include <debug.h>
#include <stdio.h>
#include <tamtypes.h> //u32 etc types
#include <loadcore.h>
#include <irx.h>
#include <types.h>
#include <unistd.h>
#include <dmaKit.h>
#include <graph.h>
#include <gsKit.h>
#include <libpad.h> //controller api

static char padBuf[256] __attribute__((aligned(64))); // pad buffer

IRX_ID("hello_iop", 1, 1);


void init_pad(void) // dualshock mode is not automatic.
{
  int ret;
  padInit(0);
  ret = padPortOpen(0, 0, padBuf);
  if (ret == 0) {
    printf("padPortOpen failed\n");
    SleepThread();
  }

  while ((ret = padGetState(0, 0)) != PAD_STATE_STABLE) {
    printf("Waiting for pad... state=%d\n", ret);
    usleep(10000);
  }

  padSetMainMode(0, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
}

struct padButtonStatus buttons;
u32 old_buttons = 0;

int main(int argc, char *argv[]) {

   init_scr();
   scr_printf("Hello World\n");


  printf("Phase 1: Input + Timing\n");
  init_pad();

  // u64 clear_color = GS_SETREG_RGBAQ(0x10, 0x10, 0x20, 0xFF, 0x00);
  // u64 color = GS_SETREG_RGBAQ(0xFF, 0x40, 0x40, 0xFF, 0x00);

  while (1) {
    int ret = padRead(0, 0, &buttons);
    if (ret > 0) {
      u32 new_buttons = buttons.btns;
      u32 pressed = ~new_buttons; // because buttons are 'active low' flip bit
      u32 changed = pressed ^ old_buttons;

      if (changed & PAD_CROSS) {
        if (pressed & PAD_CROSS)
          printf("CROSS pressed\n");
        else
          printf("CROSS released\n");
      }
      old_buttons = pressed;
    }
  }
  return MODULE_RESIDENT_END;
  }
