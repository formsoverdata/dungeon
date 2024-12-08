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

unsigned char man_frame = 1;
unsigned char direction = 0; // 0:up;1:right;2:down;3:left
unsigned char background_1;
unsigned char background_2;
unsigned char torch_size = 1;

static inline void draw_man_up(void)
{        
    switch (man_frame)
    {
        default:
        case 1:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern1); // draw man
            man_frame = 2;
            break;
        case 2:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern2); // draw man
            man_frame = 3;
            break;
        case 3:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern1); // draw man
            man_frame = 4;
            break;
        case 4:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_up_pattern3); // draw man
            man_frame = 1;
            break;
    }
}

static inline void draw_man_down(void)
{    
    switch (man_frame)
    {
        default:
        case 1:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern1); // draw man
            man_frame = 2;
            break;
        case 2:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern2); // draw man
            man_frame = 3;
            break;
        case 3:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern1); // draw man
            man_frame = 4;
            break;
        case 4:
            fill_rectangle_char(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, man_down_pattern3); // draw man
            man_frame = 1;
            break;
    }
}

static void inline hide_player(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, player_tile_next, player_tile_next);
    bright_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2);
    copy_attr_buffer();
}

static inline unsigned char get_tile(unsigned char x, unsigned char y)
{
    // no need to check if seen but rotate to last 3 (background)
    return (get_map_tile(x, y) >> 1 & 0b00000111);
}

void player_draw_up(void)
{
    switch (direction)
    {
        case 1:
        case 3:
            hide_player();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, bar_pattern);
            break;
    }
    draw_man_up();
    direction = 0;
    player_tile = player_tile_next;
    player_tile_next = get_tile(player_x - 1, player_y);
    background_1 = player_tile;
    background_2 = player_tile_next;
}

void player_draw_right(void)
{
    switch (direction)
    {
        case 0:
        case 2:
            hide_player();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, pipe_pattern);
            break;
    }
    draw_man_down();
    direction = 1;
    player_tile = player_tile_next;
    player_tile_next = get_tile(player_x, player_y + 1);
    background_1 = player_tile;
    background_2 = player_tile_next;
}

void player_draw_down(void)
{
    switch (direction)
    {
        case 1:
        case 3:
            hide_player();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, bar_pattern);
            break;
    }
    draw_man_down();
    direction = 2;
    player_tile = player_tile_next;
    player_tile_next = get_tile(player_x + 1, player_y);
    background_1 = player_tile_next;
    background_2 = player_tile;
}

void player_draw_left(void)
{
    switch (direction)
    {
        case 0:
        case 2:
            hide_player();    
            fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, pipe_pattern);
            break;
    }
    draw_man_down();
    direction = 3;
    player_tile = player_tile_next;
    player_tile_next = get_tile(player_x, player_y - 1);
    background_1 = player_tile_next;
    background_2 = player_tile;
}

void player_draw_background_vertical(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 1, 2, background_2, 6);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE, 1, 2, background_1, 7);
    bright_rectangle_attr(PLAYER_SQUARE - torch_size, PLAYER_SQUARE - torch_size, 2 + torch_size + torch_size, 2 + torch_size + torch_size);
    copy_attr_buffer();
}

void player_draw_background_horizontal(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 1, 1, background_1, 6);
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE + 1, 1, 1, background_2, 6);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE, 1, 1, background_1, 7);
    fill_rectangle_attr(PLAYER_SQUARE + 1, PLAYER_SQUARE + 1, 1, 1, background_2, 7);
    bright_rectangle_attr(PLAYER_SQUARE - torch_size, PLAYER_SQUARE - torch_size, 2 + torch_size + torch_size, 2 + torch_size + torch_size);
    copy_attr_buffer();
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
    switch (torch_size)
    {
        default:
        case 1:
            torch_size = 2;
            break;
        case 2:
            torch_size = 1;
            break;
    }
    // final background draw should have solid background
    background_1 = player_tile_next;
    background_2 = player_tile_next;
}