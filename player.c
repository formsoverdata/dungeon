#include "globals.h"

// imported from map.asm
extern unsigned char get_map_tile(unsigned char x, unsigned char y) __z88dk_callee;
extern void set_map_tile(unsigned char x, unsigned char y, unsigned int tile)  __z88dk_callee;
// imported from fill_rectangle.asm
extern unsigned char pipe_pattern[];
extern unsigned char bar_pattern[];
extern unsigned char man_up_pattern1[];
extern unsigned char man_up_pattern2[];
extern unsigned char man_up_pattern3[];
extern unsigned char man_down_pattern1[];
extern unsigned char man_down_pattern2[];
extern unsigned char man_down_pattern3[];
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void bright_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width) __z88dk_callee;
extern void copy_attr_buffer(void) __z88dk_callee; // copy attribute buffer into attribute memory

unsigned char player_frame = 1;
unsigned char player_direction = 0; // 0:up;1:right;2:down;3:left
unsigned char player_background_1;
unsigned char player_background_2;
unsigned char player_torch_size = 1;

static inline void frame_draw_up(void)
{        
    switch (player_frame)
    {
        default:
        case 1:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern1); // draw man
            player_frame = 2;
            break;
        case 2:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern2); // draw man
            player_frame = 3;
            break;
        case 3:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern1); // draw man
            player_frame = 4;
            break;
        case 4:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern3); // draw man
            player_frame = 1;
            break;
    }
}

static inline void frame_draw_down(void)
{    
    switch (player_frame)
    {
        default:
        case 1:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern1); // draw man
            player_frame = 2;
            break;
        case 2:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern2); // draw man
            player_frame = 3;
            break;
        case 3:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern1); // draw man
            player_frame = 4;
            break;
        case 4:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern3); // draw man
            player_frame = 1;
            break;
    }
}

static void inline player_hide(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, player_tile_next, player_tile_next);
    bright_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2);
    copy_attr_buffer();
}

static inline unsigned char player_get_tile(unsigned char x, unsigned char y)
{
    // no need to check if seen but rotate to last 3 (background)
    return (get_map_tile(x, y) >> 1 & 0b00000111);
}

void player_draw_background_vertical(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 1, 2, player_background_2, 6);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE, 1, 2, player_background_1, 7);
    bright_rectangle_attr(PLAYER_SQUARE - player_torch_size, PLAYER_SQUARE - player_torch_size, 2 + player_torch_size + player_torch_size, 2 + player_torch_size + player_torch_size);    
}

void player_draw_background_horizontal(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 1, 1, player_background_1, 6);
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE + 1, 1, 1, player_background_2, 6);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE, 1, 1, player_background_1, 7);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE + 1, 1, 1, player_background_2, 7);
    bright_rectangle_attr(PLAYER_SQUARE - player_torch_size, PLAYER_SQUARE - player_torch_size, 2 + player_torch_size + player_torch_size, 2 + player_torch_size + player_torch_size);
}

void player_draw_up(void)
{    
    player_tile = player_tile_next;
    player_tile_next = player_get_tile(player_x - 1, player_y);
    player_background_1 = player_tile;
    player_background_2 = player_tile_next;
    switch (player_direction)
    {
        case 0:
        case 2:        
            frame_draw_up();
            break;
        case 1:
        case 3:
            // change direction - hide player, switch between bars and pipes, draw then show player
            player_hide();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, bar_pattern);
            frame_draw_up();
            player_draw_background_vertical();
            copy_attr_buffer();
            break;
    }    
    player_direction = 0;
}

void player_draw_right(void)
{
    player_tile = player_tile_next;
    player_tile_next = player_get_tile(player_x, player_y + 1);
    player_background_1 = player_tile;
    player_background_2 = player_tile_next;
    switch (player_direction)
    {
        case 0:
        case 2:
            // change direction - hide player, switch between bars and pipes, draw then show player
            player_hide();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, pipe_pattern);
            frame_draw_down();
            player_draw_background_horizontal();
            copy_attr_buffer();
            break;
        case 1:
        case 3:
            frame_draw_down();
            break;
    }    
    player_direction = 1;    
}

void player_draw_down(void)
{
    player_tile = player_tile_next;
    player_tile_next = player_get_tile(player_x + 1, player_y);
    player_background_1 = player_tile_next;
    player_background_2 = player_tile;
    switch (player_direction)
    {
        case 0:
        case 2:
            frame_draw_down();
            break;
        case 1:
        case 3:
            // change direction - hide player, switch between bars and pipes, draw then show player
            player_hide();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, bar_pattern);
            frame_draw_down();
            player_draw_background_vertical();
            copy_attr_buffer();
            break;
    }    
    player_direction = 2;    
}

void player_draw_left(void)
{
    player_tile = player_tile_next;
    player_tile_next = player_get_tile(player_x, player_y - 1);
    player_background_1 = player_tile_next;
    player_background_2 = player_tile;
    switch (player_direction)
    {
        case 0:
        case 2:
            // change direction - hide player, switch between bars and pipes, draw then show player
            player_hide();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, pipe_pattern);
            frame_draw_down();
            player_draw_background_horizontal();
            copy_attr_buffer();
            break;
        case 1:
        case 3:
            frame_draw_down();
            break;
    }    
    player_direction = 3;    
}

void player_see(unsigned char up, unsigned char down, unsigned char left, unsigned char right)
{
    // mark area around player as seen
    for (unsigned char x = player_x + down; x >= player_x - up && x < 255; x--)
    {
        for (unsigned char y = player_y + right; y >= player_y - left && y < 255; y--)
        {
            set_map_tile(x, y, get_map_tile(x, y) | 0b00000001);
        }
    }
    // toggle the torch size
    switch (player_torch_size)
    {
        default:
        case 1:
            player_torch_size = 2;
            break;
        case 2:
            player_torch_size = 1;
            break;
    }
    // final background draw should have solid background
    player_background_1 = player_tile_next;
    player_background_2 = player_tile_next;
}