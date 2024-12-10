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
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void copy_attr_buffer(void) __z88dk_callee; // copy attribute buffer into attribute memory

void loop_around_map(void) // loop around map with 1 tile between player and edge
{
    // intial position/map
    player_x = MAP_SIZE - 2;
    player_y = 1;
    // forward    
    for (player_x = MAP_SIZE - 2; player_x > 1;)
    {
        map_move_up();
    }    
    // right    
    for (player_y = 1; player_y < MAP_SIZE - 2;)
    {
        map_move_right();
    }
    // backward    
    for (player_x = 1; player_x < MAP_SIZE - 2;)
    {
        map_move_down();
    }
    // left    
    for (player_y = MAP_SIZE - 2; player_y > 1;)
    {
        map_move_left();
    }
}

void main(void)
{       
    print_string("Initialising...");    
    map_init();    
    fill_rectangle_char(0, 0, 24, 32, " "); // repeating background pattern
    fill_rectangle_attr(0, 0, 24, 32, 0, 7);
    copy_attr_buffer();
    do
    {        
        // loop around map
        loop_around_map();
    } while (1);
}