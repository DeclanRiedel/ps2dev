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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <libpad.h>   //controller api
#include <tamtypes.h> //u32 etc types

static char padBuf[256] __attribute__((aligned(64))); // pad buffer

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
  // ###########################//
  //  Input + Timing

  printf("Phase 1: Input + Timing\n");

  init_pad();

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
      // without frame timing, the ee will run everything as fast as possible.
      // Simple vsync wait using GS register polling
      while (!(*(volatile unsigned int *)0x12000000 & 0x08));
    }
  }

  return 0;
}

  // 	GSGLOBAL *gsGlobal;
  // 	// GS_MODE_VGA_640_60
  // #ifdef HAVE_LIBTIFF
  // 	GSTEXTURE Sprite;
  //     u64 TexCol = GS_SETREG_RGBAQ(0x80,0x80,0x80,0x80,0x00);
  // #endif
  // 	u64 White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00);
  //     u64 Red = GS_SETREG_RGBAQ(0xFF,0x00,0x00,0x00,0x00);
  //     u64 Green = GS_SETREG_RGBAQ(0x00,0xFF,0x00,0x00,0x00);
  // 	u64 Blue = GS_SETREG_RGBAQ(0x00,0x00,0xFF,0x00,0x00);
  // 	u64 BlueTrans = GS_SETREG_RGBAQ(0x00,0x00,0xFF,0x40,0x00);

  //         float x = 10;
  //         float y = 10;
  //         float width = 150;
  //         float height = 150;

  //     float VHeight;

  // 	gsGlobal = gsKit_init_global();

  // 	VHeight = gsGlobal->Height;

  // 	gsGlobal->PSM = GS_PSM_CT24;
  // 	gsGlobal->PSMZ = GS_PSMZ_16S;
  // 	// gsGlobal->DoubleBuffering = GS_SETTING_OFF;
  // 	// gsGlobal->ZBuffering = GS_SETTING_OFF;

  // 	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
  // 		    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

  // 	// Initialize the DMAC
  // 	dmaKit_chan_init(DMA_CHANNEL_GIF);

  // 	gsGlobal->PrimAlphaEnable = GS_SETTING_ON;

  // 	gsKit_init_screen(gsGlobal);
  // #ifdef HAVE_LIBTIFF
  // 	Sprite.Delayed = 1;
  // 	if(gsKit_texture_tiff(gsGlobal, &Sprite, "alpha.tiff") < 0)
  // 	{
  // 		printf("Loading Failed!\n");
  // 	}
  // #endif
  // 	gsKit_mode_switch(gsGlobal, GS_PERSISTENT);

  // 	while(1)
  // 	{
  //                 if( y <= 10  && (x + width) < (gsGlobal->Width - 10))
  //                         x+=10;
  //                 else if( (y + height)  <  (VHeight - 10) && (x + width) >=
  //                 (gsGlobal->Width - 10) )
  //                         y+=10;
  //                 else if( (y + height) >=  (VHeight - 10) && x > 10 )
  //                         x-=10;
  //                 else if( y > 10 && x <= 10 )
  //                         y-=10;

  // 		gsKit_clear(gsGlobal, White);

  // 		gsKit_prim_quad_gouraud(gsGlobal, 250.0f, 50.0f, 250.0f, 400.0f,
  // 						  400.0f, 50.0f, 400.0f, 400.0f,
  // 						  1, Red, Green, Blue, White);

  // 		gsKit_prim_sprite(gsGlobal, x, y, x + width, y + height, 2,
  // BlueTrans);

  // 		gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
  // 		gsKit_set_test(gsGlobal, GS_ATEST_OFF);
  // #ifdef HAVE_LIBTIFF
  // 		gsKit_TexManager_bind(gsGlobal, &Sprite);
  // 		gsKit_prim_sprite_texture(gsGlobal, &Sprite,
  // 								100.0f,  // X1
  // 								+400.0f,  // Y2
  // 								0.0f,  // U1
  // 								0.0f,  // V1
  // 								Sprite.Width +
  // 310.0f, // X2 								Sprite.Height - 250.0f, // Y2 								Sprite.Width, // U2
  // 								Sprite.Height,
  // // V2 								3, 								TexCol); #endif 		gsKit_set_test(gsGlobal, GS_ATEST_ON);
  // 		gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);

  // 		gsKit_sync_flip(gsGlobal);

  // 		gsKit_queue_exec(gsGlobal);

  // 		gsKit_queue_reset(gsGlobal->Per_Queue);

  // 		gsKit_TexManager_nextFrame(gsGlobal);
  // 	}
  // 	return 0;
  // }
