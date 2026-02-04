#include <gsKit.h>
#include <dmaKit.h>
#include <kernel.h>
#include <math.h>

int main(int argc, char *argv[])
{
    GSGLOBAL *gsGlobal = gsKit_init_global();
    
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
    
    // Light blue for top half
    u64 sky_color = GS_SETREG_RGBAQ(100, 120, 180, 0x00, 0x00);
    
    // Ground color
    u64 ground_color = GS_SETREG_RGBAQ(120, 100, 80, 0x00, 0x00);
    
    // Dino gray
    u64 dino_color = GS_SETREG_RGBAQ(180, 180, 180, 0x00, 0x00);
    
    int frame = 0;
    float dino_y = 300.0f;
    
    while (1) {
        frame++;
        
        // Animated background color (like original working code)
        float r_sin = sinf(frame * 0.02f);
        float g_sin = sinf(frame * 0.02f + 2.094f);
        float b_sin = sinf(frame * 0.02f + 4.189f);
        
        u8 r = (u8)((r_sin * 127.0f) + 128.0f);
        u8 g = (u8)((g_sin * 127.0f) + 128.0f);
        u8 b = (u8)((b_sin * 127.0f) + 128.0f);
        
        u64 color = GS_SETREG_RGBAQ(r, g, b, 0x00, 0x00);
        
        // Clear with cycling color
        gsKit_clear(gsGlobal, color);
        
        // Draw light blue top half (sky)
        gsKit_prim_sprite(gsGlobal, 0.0f, 0.0f, 640.0f, 240.0f, 1, sky_color);
        
        // Draw ground line
        gsKit_prim_sprite(gsGlobal, 0.0f, 350.0f, 640.0f, 355.0f, 1, ground_color);
        
        // Draw dino
        gsKit_prim_sprite(gsGlobal, 100.0f, dino_y, 140.0f, dino_y + 50.0f, 1, dino_color);
        
        // Draw dino eye
        gsKit_prim_sprite(gsGlobal, 125.0f, dino_y + 10.0f, 131.0f, dino_y + 16.0f, 1, 
                         GS_SETREG_RGBAQ(0, 0, 0, 0x00, 0x00));
        
        // Flip
        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_queue_reset(gsGlobal->Per_Queue);
    }
    
    return 0;
}
