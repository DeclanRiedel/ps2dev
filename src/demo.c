#include <gsKit.h>
#include <dmaKit.h>
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

void update_game(GameState *game) {
    if (game->game_over) {
        // Auto-restart after 100 frames
        if (game->frame_count % 100 == 0) {
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
    
    // Auto-jump every 120 frames
    if (game->frame_count % 120 == 0 && game->dino.is_grounded) {
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
    
    // NO PAD INIT - Auto-play mode
    
    // Initialize game
    GameState game;
    init_game_state(&game);
    
    // Colors
    u64 sky_dark = GS_SETREG_RGBAQ(10, 20, 60, 0x00, 0x00);
    u64 sky_light = GS_SETREG_RGBAQ(10, 20, 120, 0x00, 0x00);
    u64 ground_color = GS_SETREG_RGBAQ(30, 80, 40, 0x00, 0x00);
    u64 dino_color = GS_SETREG_RGBAQ(180, 180, 180, 0x00, 0x00);
    u64 obs_color = GS_SETREG_RGBAQ(100, 60, 40, 0x00, 0x00);
    u64 star_color = GS_SETREG_RGBAQ(255, 255, 150, 0x00, 0x00);
    u64 go_color = GS_SETREG_RGBAQ(200, 50, 50, 0x00, 0x00);
    
    // Star positions (fixed 20 stars)
    float star_x[20] = {50, 150, 250, 350, 450, 550, 100, 200, 300, 400, 500, 600, 75, 175, 275, 375, 475, 575, 125, 525};
    float star_y[20] = {30, 50, 40, 60, 35, 55, 80, 90, 70, 85, 75, 95, 120, 110, 130, 115, 125, 105, 150, 145};
    
    while (1) {
        // Update game (auto-play)
        update_game(&game);
        
        // Clear screen
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(200, 200, 210, 0x00, 0x00));
        
        // Draw sky (simple 3-layer gradient)
        gsKit_prim_sprite(gsGlobal, 0.0f, 0.0f, 640.0f, 80.0f, 1, sky_dark);
        gsKit_prim_sprite(gsGlobal, 0.0f, 80.0f, 640.0f, 160.0f, 1, GS_SETREG_RGBAQ(10, 20, 90, 0x00, 0x00));
        gsKit_prim_sprite(gsGlobal, 0.0f, 160.0f, 640.0f, GROUND_Y, 1, sky_light);
        
        // Draw stars (just 20 small dots)
        for (int i = 0; i < 20; i++) {
            gsKit_prim_sprite(gsGlobal, star_x[i], star_y[i], star_x[i] + 2.0f, star_y[i] + 2.0f, 1, star_color);
        }
        
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
        
        // Simple score indicator (white box)
        gsKit_prim_sprite(gsGlobal, 500.0f, 20.0f, 620.0f, 60.0f, 1, 
                         GS_SETREG_RGBAQ(255, 255, 255, 0x00, 0x00));
        
        // Game over text
        if (game.game_over) {
            // Draw big red box
            gsKit_prim_sprite(gsGlobal, 200.0f, 200.0f, 440.0f, 280.0f, 1, go_color);
        }
        
        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_queue_reset(gsGlobal->Per_Queue);
    }
    
    return 0;
}
