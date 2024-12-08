#include <stdlib.h>
#include "globals.h"
#include "player.h"

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
unsigned char map_frame;

static inline unsigned char row_get_tile(unsigned char x, unsigned char y)
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

void row_draw_vertical(signed char x, signed char x2, unsigned char y)
{
    unsigned char i = VISIBLE_BLOCKS;
    for (; i > 0; i--)
    {
        unsigned char tile = row_get_tile(x, y);
        unsigned char tile2 = x == x2 ? tile : row_get_tile(x2, y);
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

void row_draw_horizontal(signed char x, unsigned char y)
{    
    for (unsigned char i = VISIBLE_BLOCKS; i < 255; i--)
    {
        unsigned char tile = row_get_tile(x, y);
        unsigned char tile2 = tile;
        switch (map_frame)
        {
            case 1:
            case 3:
                tile = row_get_tile(x, y - 1);
                break;
        }

        if (i == 0)
        {
            switch (map_frame)
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
            if (map_frame == 0 || map_frame == 3 || i < VISIBLE_BLOCKS)
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

void map_draw_vertical(void)
{    
    attr_address = start_attr_address; // reset shared attr_address    
    unsigned char sub_frame = 0;
    switch (map_frame)
    {
        case 1:
        case 3:
            sub_frame++;
            break;
    }
    signed char x = player_x - MAP_OFFSET - 1; // starting row (could be negative)
    unsigned char y = player_y - MAP_OFFSET;
    switch (map_frame)
    {
        case 2:
        case 3:
            row_draw_vertical(x, x, y);
            break;
    }
    signed char rows = player_x + MAP_OFFSET;
    for (x = x + 1; x < rows; x++)
    {
        row_draw_vertical(x - sub_frame, x, y);
        row_draw_vertical(x, x, y);
    }
    row_draw_vertical(x - sub_frame, x, y);
    if (map_frame < 2)
    {
        // catch up row
        row_draw_vertical(x, x, y);
    }    
}

void map_draw_horizontal(void)
{
    attr_address = start_attr_address; // reset shared attr_address    
    unsigned char y = player_y - MAP_OFFSET;
    if (map_frame == 2)
    {
        y--;
    }
    for (signed char x = player_x - MAP_OFFSET; x <= player_x + MAP_OFFSET; x++)
    {
        row_draw_horizontal(x, y);
        row_draw_horizontal(x, y);
    }    
}

void map_init(void)
{
    start_attr_address = (unsigned char*)(ATTR_BUFF); // start of map attribute memory
    for (unsigned char x = MAP_SIZE - 1; x < 255; x--)
    {
        for (unsigned char y = MAP_SIZE - 1; y < 255 ; y--)
        {
            set_map_tile(x, y, (rand() % 6) << 1);
        }
    }
}

void map_draw(void)
{
    player_draw_down();
    player_see(2, 2, 2, 2);
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
}

void map_move_up(void)
{       
    player_draw_up();
    // animate up
    map_frame = 1;    
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
    map_frame++;
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
    map_frame++;
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
    player_x--;    
    map_frame = 0;
    player_see(3, 2, 2, 2); // final position    
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
}

void map_move_down(void)
{        
    player_draw_down();
    // animate down
    player_x++;
    map_frame = 3;    
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
    map_frame--;
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
    map_frame--;
    map_draw_vertical(); 
    player_draw_background_vertical();
    copy_attr_buffer();
    map_frame--;
    player_see(2, 3, 2, 2); // final position    
    map_draw_vertical();
    player_draw_background_vertical();
    copy_attr_buffer();
}

void map_move_left(void)
{   
    player_draw_left();
    // animate left
    map_frame = 1;    
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    map_frame++;
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    map_frame++;
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    player_y--;
    map_frame = 0;
    player_see(2, 2, 3, 2); // final position    
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
}

void map_move_right(void)
{   
    player_draw_right();
    // animate right 
    player_y++;
    map_frame = 3;    
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    map_frame--;
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    map_frame--;
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
    map_frame--;
    player_see(2, 2, 2, 3); // final position
    map_draw_horizontal();
    player_draw_background_horizontal();
    copy_attr_buffer();
}