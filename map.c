#include <stdlib.h>
#include "globals.h"

// imported from map.asm
extern unsigned char get_map_tile(unsigned char x, unsigned char y) __z88dk_callee;
extern void set_map_tile(unsigned char x, unsigned char y, unsigned int tile)  __z88dk_callee;
// imported from fill_rectangle.asm
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void bright_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width) __z88dk_callee;
extern void copy_attr_buffer(void) __z88dk_callee; // copy attribute buffer into attribute memory

unsigned char *start_attr_address;
unsigned char *attr_address;
unsigned char frame_no;
unsigned char torch_size = 1;

static inline unsigned char get_tile(unsigned char x, unsigned char y)
{
    if (x < MAP_SIZE && y < MAP_SIZE)
    {
        unsigned char tile = get_map_tile(x, y);        
        if (tile & 0b00000001 == 0b00000001) // has tile been seen
        {
            return (tile << 2 & 0b00111000);
        }
    }

    return 0;
}

static inline void player_see(unsigned char up, unsigned char down, unsigned char left, unsigned char right)
{
    // mark area around player as seen
    for (unsigned char x = player_x + down; x >= player_x - up && x < 255; x--)
    {
        for (unsigned char y = player_y + right; y >= player_y - left && y < 255; y--)
        {
            set_map_tile(x, y, get_map_tile(x, y) | 0b00000001);
        }
    }
}

void draw_row_vertical(signed char x, signed char x2, unsigned char y)
{
    unsigned char i = VISIBLE_BLOCKS;
    for (; i > 0; i--)
    {
        unsigned char tile = get_tile(x, y);
        unsigned char tile2 = x == x2 ? tile : get_tile(x2, y);
        tile = tile >> 3 | tile2;
        *attr_address++ = tile;
        *attr_address++ = tile;
        y++;
    }
    for (i = 15; i >= VISIBLE_BLOCKS; i--)
    {
        attr_address++;
        attr_address++;
    }
}

void draw_row_horizontal(signed char x, unsigned char y)
{    
    for (unsigned char i = VISIBLE_BLOCKS; i < 255; i--)
    {
        unsigned char tile = get_tile(x, y);
        unsigned char tile2 = tile;
        switch (frame_no)
        {
            case 1:
            case 3:
                tile = get_tile(x, y - 1);
                break;
        }

        if (i == 0)
        {
            switch (frame_no)
            {
                case 1:
                case 2:
                    // catch up last block (first skipped)
                    *attr_address++ = tile >> 3 | tile;
                    break;
            }
            attr_address++;
            attr_address++;
        }
        else
        {
            if (frame_no == 0 || frame_no == 3 || i < VISIBLE_BLOCKS)
            {
                // do not skip first block
                *attr_address++ = tile >> 3 | tile;
            }
            *attr_address++ = tile >> 3 | tile2;
            y++;
        }
    }
    for (unsigned char i = 15; i > VISIBLE_BLOCKS; i--)
    {        
        attr_address++;
        attr_address++;
    }
}

void draw_map_vertical(void)
{    
    attr_address = start_attr_address; // reset shared attr_address    
    unsigned char sub_frame = 0;
    switch (frame_no)
    {
        case 1:
        case 3:
            sub_frame++;
            break;
    }
    signed char x = player_x - MAP_OFFSET - 1; // starting row (could be negative)
    unsigned char y = player_y - MAP_OFFSET;
    switch (frame_no)
    {
        case 2:
        case 3:
            draw_row_vertical(x, x, y);
            break;
    }
    signed char rows = player_x + MAP_OFFSET;
    for (x = x + 1; x < rows; x++)
    {
        draw_row_vertical(x - sub_frame, x, y);
        draw_row_vertical(x, x, y);
    }
    draw_row_vertical(x - sub_frame, x, y);
    if (frame_no < 2)
    {
        // catch up row
        draw_row_vertical(x, x, y);
    }
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, 7, 7);
    bright_rectangle_attr(PLAYER_SQUARE - torch_size, PLAYER_SQUARE - torch_size, 2 + torch_size + torch_size, 2 + torch_size + torch_size);
    copy_attr_buffer();
}

void draw_map_horizontal(void)
{
    attr_address = start_attr_address; // reset shared attr_address    
    unsigned char y = player_y - MAP_OFFSET;
    if (frame_no == 2)
    {
        y--;
    }
    for (signed char x = player_x - MAP_OFFSET; x <= player_x + MAP_OFFSET; x++)
    {
        draw_row_horizontal(x, y);
        draw_row_horizontal(x, y);
    }
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, 7, 7);
    bright_rectangle_attr(PLAYER_SQUARE - torch_size, PLAYER_SQUARE - torch_size, 2 + torch_size + torch_size, 2 + torch_size + torch_size);
    copy_attr_buffer();
}

void init_map(void)
{
    start_attr_address = (unsigned char*)(ATTR_BUFF); // start of map attribute memory
    for (unsigned char x = MAP_SIZE - 1; x < 255; x--)
    {
        for (unsigned char y = MAP_SIZE - 1; y < 255 ; y--)
        {
            set_map_tile(x, y, (rand() % 7) << 1);
        }
    }
}

static inline void toggle_torch_size(void)
{
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
}

void draw_map(void)
{
    player_see(2, 2, 2, 2);
    draw_map_vertical();
}

void move_up(void)
{   
    // animate up
    frame_no = 1;    
    draw_map_vertical();
    frame_no++;
    draw_map_vertical();
    frame_no++;
    draw_map_vertical();
    player_x--;    
    frame_no = 0;
    player_see(3, 2, 2, 2);
    draw_map_vertical(); // final position
    toggle_torch_size();
}

void move_down(void)
{        
    // animate down
    player_x++;
    frame_no = 3;
    draw_map_vertical();
    frame_no--;
    draw_map_vertical();
    frame_no--;
    draw_map_vertical(); 
    frame_no--;
    player_see(2, 3, 2, 2);
    draw_map_vertical(); // final position
    toggle_torch_size();
}

void move_left(void)
{   
    // animate left
    frame_no = 1;
    draw_map_horizontal();
    frame_no++;
    draw_map_horizontal();
    frame_no++;
    draw_map_horizontal(); 
    player_y--;
    frame_no = 0;
    player_see(2, 2, 3, 2);
    draw_map_horizontal(); // final position
    toggle_torch_size();
}

void move_right(void)
{   
    // animate right 
    player_y++;
    frame_no = 3;
    draw_map_horizontal();
    frame_no--;
    draw_map_horizontal();
    frame_no--;
    draw_map_horizontal();
    frame_no--;
    player_see(2, 2, 2, 3);
    draw_map_horizontal(); // final position
    toggle_torch_size();
}