/*
NOTE - We are using newlib which defines sections for our ASM, not using these as appropriate seems to cause strange issues
SECTION code_user: assign executable code to this section
SECTION rodata_user: assign read-only data to this section
SECTION smc_user: assign self-modifying code to this section
SECTION data_user: assign non-zero initial data to this section
SECTION bss_user: assign zero initial data to this section
*/

#include <stdlib.h>
#include "globals.h"
#include "map.h"

// imported from screen.asm
extern void print_string(uint8_t *string) __z88dk_fastcall; // print null terminated string, accepts rst $10 control codes

// imported from fill_rectangle.asm
extern unsigned char background_pattern1[];
extern unsigned char background_pattern2[];
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void bright_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width) __z88dk_callee;
extern void copy_attr_buffer(void) __z88dk_callee; // copy attribute buffer into attribute memory

static void inline hide_player(void)
{
    fill_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2, player_tile, player_tile);
    bright_rectangle_attr(PLAYER_SQUARE, PLAYER_SQUARE, 2, 2);
    copy_attr_buffer();
}

void loop_around_map(void) // loop around map with 1 tile between player and edge
{
    // intial position/map
    player_x = MAP_SIZE - 2;
    player_y = 1;
    draw_map();
    // forward
    hide_player();    
    fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, "["); // horizontal stripe
    for (player_x = MAP_SIZE - 2; player_x > 1;)
    {
        move_up();
    }    
    // right
    hide_player();
    fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, "\\"); // vertical stripe    
    for (player_y = 1; player_y < MAP_SIZE - 2;)
    {
        move_right();
    }
    // backward
    hide_player();
    fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, "["); // horizontal stripe
    for (player_x = 1; player_x < MAP_SIZE - 2;)
    {
        move_down();
    }
    // left
    hide_player();
    fill_rectangle_char(0, 0, VISIBLE_AREA, VISIBLE_AREA, "\\"); // vertical stripe
    for (player_y = MAP_SIZE - 2; player_y > 1;)
    {
        move_left();
    }
}

void main(void)
{       
    print_string("Initialising...");    
    init_map();    
    // try out some fill_rectangle stuff
    /*fill_rectangle_char(0, 0, 24, 32, background_pattern1); // repeating background pattern
    fill_rectangle_attr(0, 0, 24, 32, 7, 1);
    copy_attr_buffer();
    fill_rectangle_char(0, 0, 24, 32, background_pattern2); // repeating background pattern              
    fill_rectangle_attr(0, 0, 24, 32, 2, 4);
    copy_attr_buffer();
    fill_rectangle_char(1, 1, 22, 30, "HELLO "); // can also enter text directly
    fill_rectangle_attr(1, 1, 22, 30, 6, 2);
    copy_attr_buffer();
    fill_rectangle_char(2, 2, 20, 28, "THERE ");
    fill_rectangle_attr(2, 2, 20, 28, 5, 3);
    copy_attr_buffer();
    fill_rectangle_char(3, 3, 18, 26, "HOW ");
    fill_rectangle_attr(3, 3, 18, 26, 4, 0);
    copy_attr_buffer();
    fill_rectangle_char(4, 4, 16, 24, "ARE ");
    fill_rectangle_attr(4, 4, 16, 24, 3, 5);
    copy_attr_buffer();
    fill_rectangle_char(5, 5, 14, 22, "YOU ");
    fill_rectangle_attr(5, 5, 14, 22, 2, 6);
    copy_attr_buffer();*/
    fill_rectangle_char(0, 0, 24, 32, " "); // repeating background pattern
    fill_rectangle_attr(0, 0, 24, 32, 0, 7);
    copy_attr_buffer();
    do
    {        
        // loop around map
        loop_around_map();
    } while (1);
}