#include <stdlib.h>
#include "globals.h"

// imported from map.asm
extern unsigned char get_map_tile(unsigned char x, unsigned char y) __z88dk_callee;
extern void set_map_tile(unsigned char x, unsigned char y, unsigned int tile)  __z88dk_callee;
// imported from fill_rectangle.asm
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void copy_attr_buffer(void) __z88dk_callee; // copy attribute buffer into attribute memory

unsigned char *start_attr_address;
unsigned char *attr_address;
unsigned char frame_no;

unsigned char get_tile(unsigned char x, unsigned char y)
{
    if (x < MAP_SIZE && y < MAP_SIZE)
        return get_map_tile(x, y);

    return 0;
}

void draw_row_vertical(signed char x, signed char x2, unsigned char y)
{
    // tile attribute is half x, half x2, split by ink/paper using udg $83    
    for (unsigned char ty = 0; ty <= 15; ty++)
    {
        if (ty < VISIBLE_BLOCKS)
        {            
            unsigned char tile = get_tile(x, y);
            unsigned char tile2 = x == x2 ? tile : get_tile(x2, y);
            tile = tile >> 3 | tile2;
            *attr_address++ = tile;
            *attr_address++ = tile;
        }
        else
        {
            attr_address++;
            attr_address++;
        }
        y++;
    }
}

void draw_row_horizontal(signed char x, unsigned char y)
{      
    for (unsigned char ty = 0; ty <= 15; ty++)
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
        if (ty < VISIBLE_BLOCKS)
        {            
            if (frame_no == 0 || frame_no == 3 || ty > 0)
            {
                // do not skip first block
                *attr_address++ = tile >> 3 | tile;
            }
            *attr_address++ = tile >> 3 | tile2;
        }
        else
        {
            if (ty == VISIBLE_BLOCKS)
            {
                switch (frame_no)
                {
                    case 1:
                    case 2:
                        // catch up last block (first skipped)
                        *attr_address++ = tile >> 3 | tile;
                        break;
                }
                
            }
            attr_address++;
            attr_address++;
        }
        y++;
    }
}

void draw_map_vertical(void)
{    
    attr_address = start_attr_address; // reset shared attr_address
    signed char x = player_x - MAP_OFFSET; // starting row (could be negative)
    unsigned char y = player_y - MAP_OFFSET;
    unsigned char sub_frame = 0;
    switch (frame_no)
    {
        case 1:
            sub_frame = 1;
            break;
        case 2:
            draw_row_vertical(x - 1, x - 1, y);
            break;
        case 3:
            sub_frame = 1;
            draw_row_vertical(x - 1, x - 1, y);
            break;

    }

    signed char rows = player_x + MAP_OFFSET;
    while (x < rows)
    {
        draw_row_vertical(x - sub_frame, x, y);
        draw_row_vertical(x, x, y);
        x++;
    }
    draw_row_vertical(x - sub_frame, x, y);
    if (frame_no < 2)
    {
        // catch up row
        draw_row_vertical(x, x, y);
    }

    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, 7, 7); // player square
    copy_attr_buffer();
}

void draw_map_horizontal(void)
{
    attr_address = start_attr_address; // reset shared attr_address
    signed char x = player_x - MAP_OFFSET; // starting row (could be negative)
    unsigned char y = player_y - MAP_OFFSET;
    if (frame_no == 2)
    {
        y--;
    }
    while (x <= player_x + MAP_OFFSET)
    {
        draw_row_horizontal(x, y);
        draw_row_horizontal(x, y);
        x++;
    }
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, 7, 7); // player square
    copy_attr_buffer();
}

void init_map(void)
{
    start_attr_address = (unsigned char*)(ATTR_BUFF); // start of map attribute memory
    for (unsigned char x = 0; x < MAP_SIZE; x++)
        for (unsigned char y = 0; y < MAP_SIZE; y++)
            set_map_tile(x, y, (rand() % 7) << 3);
}

void move_forward(void)
{   
    // animate forward
    frame_no = 1;
    draw_map_vertical(); 
    frame_no++;
    draw_map_vertical();
    frame_no++;
    draw_map_vertical();
    player_x--;
    frame_no = 0;
    draw_map_vertical(); // final position
}

void move_backward(void)
{        
    // animate backward
    player_x++;
    frame_no = 3;
    draw_map_vertical();
    frame_no--;
    draw_map_vertical();
    frame_no--;
    draw_map_vertical(); 
    frame_no--;
    draw_map_vertical(); // final position
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
    draw_map_horizontal(); // final position
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
    draw_map_horizontal(); // final position
}