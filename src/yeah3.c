#include <gsKit.h>
#include <dmaKit.h>
#include <libpad.h>
#include <kernel.h>
#include <math.h>
#include <stdlib.h>
#include <tamtypes.h>
#include <stdio.h>

#define GROUND_Y 350
#define DINO_WIDTH 40
#define DINO_HEIGHT 50
#define JUMP_FORCE -12.0f
#define GRAVITY 0.6f
#define MAX_OBSTACLES 5

typedef struct {
    float x, y;
    float vy;
    int is_jumping;
    int is_grounded;
} Dino;

typedef struct {
    float x, y;
    int width, height;
    int active;
} Obstacle;

typedef struct {
    Dino dino;
    Obstacle obstacles[MAX_OBSTACLES];
    int score;
    int game_over;
    int frame_count;
    float game_speed;
} GameState;

void init_game_state(GameState *game) {
    game->dino.x = 100.0f;
    game->dino.y = GROUND_Y - DINO_HEIGHT;
    game->dino.vy = 0.0f;
    game->dino.is_jumping = 0;
    game->dino.is_grounded = 1;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles[i].active = 0;
    }
    
    game->score = 0;
    game->game_over = 0;
    game->frame_count = 0;
    game->game_speed = 5.0f;
}

void spawn_obstacle(GameState *game) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!game->obstacles[i].active) {
            game->obstacles[i].x = 640.0f + (float)(rand() % 100);
            game->obstacles[i].y = GROUND_Y - 40;
            game->obstacles[i].width = 20 + (rand() % 20);
            game->obstacles[i].height = 30 + (rand() % 30);
            game->obstacles[i].active = 1;
            break;
        }
    }
}

int check_collision(Dino *dino, Obstacle *obs) {
    if (!obs->active) return 0;
    
    float dino_left = dino->x + 5;
    float dino_right = dino->x + DINO_WIDTH - 5;
    float dino_top = dino->y + 5;
    float dino_bottom = dino->y + DINO_HEIGHT - 5;
    
    float obs_left = obs->x + 2;
    float obs_right = obs->x + obs->width - 2;
    float obs_top = obs->y + 2;
    float obs_bottom = obs->y + obs->height - 2;
    
    return (dino_left < obs_right && dino_right > obs_left &&
            dino_top < obs_bottom && dino_bottom > obs_top);
}

void update_game(GameState *game, u32 paddata) {
    if (game->game_over) {
        if (paddata & PAD_CROSS) {
            init_game_state(game);
        }
        return;
    }
    
    game->frame_count++;
    game->score = game->frame_count / 5;
    
    if (game->frame_count % 500 == 0) {
        game->game_speed += 0.5f;
        if (game->game_speed > 15.0f) game->game_speed = 15.0f;
    }
    
    if ((paddata & PAD_CROSS) && game->dino.is_grounded) {
        game->dino.vy = JUMP_FORCE;
        game->dino.is_jumping = 1;
        game->dino.is_grounded = 0;
    }
    
    game->dino.vy += GRAVITY;
    game->dino.y += game->dino.vy;
    
    if (game->dino.y >= GROUND_Y - DINO_HEIGHT) {
        game->dino.y = GROUND_Y - DINO_HEIGHT;
        game->dino.vy = 0.0f;
        game->dino.is_jumping = 0;
        game->dino.is_grounded = 1;
    }
    
    if (game->frame_count % (int)(120 - game->game_speed * 5) == 0) {
        if (rand() % 3 == 0) {
            spawn_obstacle(game);
        }
    }
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].active) {
            game->obstacles[i].x -= game->game_speed;
            
            if (game->obstacles[i].x < -50) {
                game->obstacles[i].active = 0;
            }
            
            if (check_collision(&game->dino, &game->obstacles[i])) {
                game->game_over = 1;
            }
        }
    }
}

// Stars for the sky
typedef struct {
    float x, y;
    u8 brightness;
} Star;

Star stars[50];

void init_stars(void) {
    for (int i = 0; i < 50; i++) {
        stars[i].x = (float)(rand() % 640);
        stars[i].y = (float)(rand() % 240);
        stars[i].brightness = 150 + (rand() % 105);
    }
}

void draw_sky(GSGLOBAL *gsGlobal) {
    // Static gradient sky - dark blue at top, lighter at horizon
    for (int y = 0; y < 240; y += 8) {
        float t = (float)y / 240.0f;
        u8 blue = (u8)(60 + t * 80);
        u64 sky_color = GS_SETREG_RGBAQ(10, 20, blue, 0x00, 0x00);
        gsKit_prim_sprite(gsGlobal, 0.0f, (float)y, 640.0f, (float)y + 8.0f, 1, sky_color);
    }
    
    // Draw stars
    for (int i = 0; i < 50; i++) {
        u64 star_color = GS_SETREG_RGBAQ(stars[i].brightness, stars[i].brightness, 100, 0x00, 0x00);
        gsKit_prim_sprite(gsGlobal, stars[i].x, stars[i].y, stars[i].x + 2.0f, stars[i].y + 2.0f, 1, star_color);
    }
}

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
    
    // Initialize pad
    padInit(0);
    
    int port = 0, slot = 0;
    static unsigned char padArea[256] __attribute__((aligned(64)));
    
    padPortOpen(port, slot, padArea);
    
    // Small delay for pad init
    for (int i = 0; i < 1000; i++) { }
    
    // Initialize game
    GameState game;
    init_game_state(&game);
    
    // Initialize stars
    init_stars();
    
    // Colors
    u64 ground_color = GS_SETREG_RGBAQ(30, 80, 40, 0x00, 0x00);
    u64 dino_color = GS_SETREG_RGBAQ(180, 180, 180, 0x00, 0x00);
    u64 obs_color = GS_SETREG_RGBAQ(100, 60, 40, 0x00, 0x00);
    u64 text_color = GS_SETREG_RGBAQ(50, 50, 50, 0x00, 0x00);
    
    int frame = 0;
    
    while (1) {
        frame++;
        
        // Read pad
        struct padButtonStatus buttons;
        padRead(port, slot, &buttons);
        u32 paddata = 0xFFFF ^ buttons.btns;
        
        // Update game
        update_game(&game, paddata);
        
        // Clear screen
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(200, 200, 210, 0x00, 0x00));
        
        // Draw sky with gradient and stars
        draw_sky(gsGlobal);
        
        // Draw solid dark green ground
        gsKit_prim_sprite(gsGlobal, 0.0f, GROUND_Y, 640.0f, 480.0f, 1, ground_color);
        
        // Draw obstacles (boxes)
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (game.obstacles[i].active) {
                gsKit_prim_sprite(gsGlobal,
                                 game.obstacles[i].x, game.obstacles[i].y,
                                 game.obstacles[i].x + game.obstacles[i].width,
                                 game.obstacles[i].y + game.obstacles[i].height,
                                 1, obs_color);
            }
        }
        
        // Draw dino
        gsKit_prim_sprite(gsGlobal, game.dino.x, game.dino.y,
                         game.dino.x + DINO_WIDTH, game.dino.y + DINO_HEIGHT, 1, dino_color);
        
        // Draw dino eye
        gsKit_prim_sprite(gsGlobal, game.dino.x + DINO_WIDTH - 10, game.dino.y + 10.0f,
                         game.dino.x + DINO_WIDTH - 4, game.dino.y + 16.0f, 1, 
                         GS_SETREG_RGBAQ(0, 0, 0, 0x00, 0x00));
        
        // Draw score (simple digit blocks)
        int score = game.score;
        int digits[6] = {0};
        int num_digits = 0;
        if (score == 0) {
            digits[0] = 0;
            num_digits = 1;
        } else {
            while (score > 0 && num_digits < 6) {
                digits[num_digits++] = score % 10;
                score /= 10;
            }
        }
        
        float score_x = 500.0f;
        for (int i = num_digits - 1; i >= 0; i--) {
            int digit = digits[i];
            // Simple 3x5 digit patterns
            for (int row = 0; row < 5; row++) {
                for (int col = 0; col < 3; col++) {
                    int draw = 0;
                    switch (digit) {
                        case 0: draw = (row == 0 || row == 4 || col == 0 || col == 2); break;
                        case 1: draw = (col == 1); break;
                        case 2: draw = (row == 0 || row == 2 || row == 4 || 
                                      (col == 2 && row < 2) || (col == 0 && row > 2)); break;
                        case 3: draw = (row == 0 || row == 2 || row == 4 || col == 2); break;
                        case 4: draw = (col == 1 || row == 2 || (col == 2 && row < 2)); break;
                        case 5: draw = (row == 0 || row == 2 || row == 4 || 
                                      (col == 0 && row < 2) || (col == 2 && row > 2)); break;
                        case 6: draw = (row == 0 || row == 2 || row == 4 || col == 0 || 
                                      (col == 2 && row > 2)); break;
                        case 7: draw = (row == 0 || col == 2); break;
                        case 8: draw = (row == 0 || row == 2 || row == 4 || col == 0 || col == 2); break;
                        case 9: draw = (row == 0 || row == 2 || row == 4 || 
                                      (col == 0 && row < 2) || col == 2); break;
                    }
                    if (draw) {
                        gsKit_prim_sprite(gsGlobal, score_x + col * 8, 20.0f + row * 8,
                                         score_x + col * 8 + 6, 20.0f + row * 8 + 6, 1, text_color);
                    }
                }
            }
            score_x += 25.0f;
        }
        
        // Game over text
        if (game.game_over) {
            u64 go_color = GS_SETREG_RGBAQ(200, 50, 50, 0x00, 0x00);
            // Draw "GAME" blocks
            for (int i = 0; i < 4; i++) {
                gsKit_prim_sprite(gsGlobal, 220.0f + i * 50, 200.0f,
                                 220.0f + i * 50 + 40, 240.0f, 1, go_color);
            }
            // "OVER" blocks
            for (int i = 0; i < 4; i++) {
                gsKit_prim_sprite(gsGlobal, 220.0f + i * 50, 250.0f,
                                 220.0f + i * 50 + 40, 290.0f, 1, go_color);
            }
        }
        
        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_queue_reset(gsGlobal->Per_Queue);
    }
    
    padPortClose(port, slot);
    
    return 0;
}
