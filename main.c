#include <gsKit.h>
#include <dmaKit.h>
#include <math.h>

int main(int argc, char *argv[])
{
    GSGLOBAL *gsGlobal = gsKit_init_global();
    u64 color;
    
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, 
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    
    gsGlobal->PSM = GS_PSM_CT24;
    gsGlobal->PSMZ = GS_PSMZ_16S;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->ZBuffering = GS_SETTING_OFF;
    gsGlobal->PrimAAEnable = GS_SETTING_OFF;
    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
    
    gsKit_init_screen(gsGlobal);
    
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);
    gsKit_set_test(gsGlobal, GS_ZTEST_OFF);
    
    int frame = 0;
    float speed = 0.02f;
    
    while (1) {
        frame++;
        
        float r_sin = sinf(frame * speed);
        float g_sin = sinf(frame * speed + 2.094f);
        float b_sin = sinf(frame * speed + 4.189f);
        
        u8 r = (u8)((r_sin * 127.0f) + 128.0f);
        u8 g = (u8)((g_sin * 127.0f) + 128.0f);
        u8 b = (u8)((b_sin * 127.0f) + 128.0f);
        
        color = GS_SETREG_RGBAQ(r, g, b, 0x00, 0x00);
        
        // Clear screen with color
        gsKit_clear(gsGlobal, color);
        
        // Draw a rectangle to ensure rendering
        gsKit_prim_sprite(gsGlobal, 50.0f, 50.0f, 
                         (float)gsGlobal->Width - 50.0f, 
                         (float)gsGlobal->Height - 50.0f, 1, color);
        
        // Flip BEFORE exec (important!)
        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_queue_reset(gsGlobal->Per_Queue);
    }
    
    return 0;
}
