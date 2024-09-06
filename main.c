/*
NOTE - We are using newlib which defines sections for our ASM, not using these as appropriate seems to cause strange issues
SECTION code_user: assign executable code to this section
SECTION rodata_user: assign read-only data to this section
SECTION smc_user: assign self-modifying code to this section
SECTION data_user: assign non-zero initial data to this section
SECTION bss_user: assign zero initial data to this section
*/

#include <stdlib.h>

// imported from screen.asm
extern void print_string(uint8_t *string) __z88dk_fastcall; // print null terminated string, accepts rst $10 control codes
// imported from map.asm
extern unsigned char get_map_tile(unsigned char x, unsigned char y) __z88dk_callee;
extern void set_map_tile(unsigned char x, unsigned char y, unsigned int tile)  __z88dk_callee;
// imported from fill_rectangle.asm
extern unsigned char background_pattern1[];
extern unsigned char background_pattern2[];
extern void fill_rectangle_char(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char *c) __z88dk_callee;
extern void fill_rectangle_attr(unsigned char x, unsigned char y, unsigned char height, unsigned char width, unsigned char ink, unsigned char paper) __z88dk_callee;
extern void copy_attr_buffer() __z88dk_callee; // copy attribute buffer into attribute memory

#define ATTR_BUFF 0xF800 // hard coded attribute buffer address
#define MAP_SIZE 64 // 64x64 map
#define VISIBLE_BLOCKS 11 // 11x11 displayed
#define MAP_OFFSET 5 // offset to centralise display

unsigned char player_x;
unsigned char player_y;

unsigned char *start_attr_address;
unsigned char *attr_address;

void init_map()
{
    for (unsigned char x = 0; x < MAP_SIZE; x++)
        for (unsigned char y = 0; y < MAP_SIZE; y++)
            set_map_tile(x, y, (rand() % 7) << 3);
}

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

void draw_row_horizontal(signed char x, unsigned char y, unsigned char y2, unsigned char frame)
{      
    for (unsigned char ty = 0; ty <= 15; ty++)
    {                
        if (ty < VISIBLE_BLOCKS)
        {
            unsigned char tile = get_tile(x, y);
            unsigned char tile2 = y == y2 ? tile : get_tile(x, y2);
            if (frame == 0 || ty > 0)
            {
                *attr_address++ = tile >> 3 | tile;
            }
            *attr_address++ = tile >> 3 | tile2;
        }
        else
        {
            if (frame == 1 && ty == VISIBLE_BLOCKS)
            {
                unsigned char tile = get_tile(x, y);
                *attr_address++ = tile >> 3 | tile;
            }
            attr_address++;
            attr_address++;
        }
        y++;
        y2++;        
    }
}

void draw_map_vertical(unsigned char frame, unsigned char sub_frame, unsigned char px, unsigned char py)
{    
    attr_address = start_attr_address; // reset shared attr_address
    signed char x = px - MAP_OFFSET - frame; // starting row (could be negative)
    unsigned char y = py - MAP_OFFSET;
    if (frame == 1)
    {
        // draw top half row between frames
        draw_row_vertical(x, x, y);
        x++;
    }

    while (x <= px + MAP_OFFSET)
    {
        draw_row_vertical(x - sub_frame, x, y);
        draw_row_vertical(x, x, y);
        x++;
    }

    fill_rectangle_attr(10, 10, 2, 2, 7, 7); // player square
    fill_rectangle_attr(22, 0, 1, 32, 1, 7); // hide bottom row
    copy_attr_buffer();
}

void draw_map_horizontal(unsigned char frame, unsigned char sub_frame, unsigned char px, unsigned char py)
{
    attr_address = start_attr_address; // reset shared attr_address
    signed char x = px - MAP_OFFSET; // starting row (could be negative)
    unsigned char y = py - MAP_OFFSET - frame;

    while (x <= px + MAP_OFFSET)
    {
        draw_row_horizontal(x, y - sub_frame, y, frame);
        draw_row_horizontal(x, y - sub_frame, y, frame);
        x++;
    }

    fill_rectangle_attr(10, 10, 2, 2, 7, 7); // player square
    fill_rectangle_attr(22, 0, 1, 32, 1, 7); // hide bottom row
    copy_attr_buffer();
}

void move_forward()
{
    unsigned char prev_x = player_x;
    player_x--;
    // animate forward
    draw_map_vertical(0, 1, prev_x, player_y); 
    draw_map_vertical(1, 0, prev_x, player_y);
    draw_map_vertical(1, 1, prev_x, player_y);
    draw_map_vertical(0, 0, player_x, player_y); // final position
}

void move_backward()
{    
    player_x++;
    // animate backward
    draw_map_vertical(1, 1, player_x, player_y);
    draw_map_vertical(1, 0, player_x, player_y);
    draw_map_vertical(0, 1, player_x, player_y); 
    draw_map_vertical(0, 0, player_x, player_y); // final position
}

void move_left()
{
    unsigned char prev_y = player_y;
    player_y--;
    // animate left
    draw_map_horizontal(1, 1, player_x, prev_y + 1); // TODO - why + 1?
    draw_map_horizontal(1, 0, player_x, prev_y );
    draw_map_horizontal(0, 1, player_x, prev_y); 
    draw_map_horizontal(0, 0, player_x, player_y); // final position
}

void move_right()
{    
    player_y++;
    // animate right
    draw_map_horizontal(0, 1, player_x, player_y);
    draw_map_horizontal(1, 0, player_x, player_y);
    draw_map_horizontal(1, 1, player_x, player_y + 1); // TODO - why + 1?
    draw_map_horizontal(0, 0, player_x, player_y); // final position
}

void loop_around_map() // loop around map with 1 tile between player and edge
{
    // intial position/map
    player_x = MAP_SIZE - 2;
    player_y = 1;
    draw_map_vertical(0, 0, player_x, player_y);
    // forward
    fill_rectangle_char(0, 0, 22, 22, "["); // horizontal stripe
    for (player_x = MAP_SIZE - 2; player_x > 1;)
    {
        move_forward();
    }    
    // right
    fill_rectangle_char(0, 0, 22, 22, "\\"); // vertical stripe
    for (player_y = 1; player_y < MAP_SIZE - 2;)
    {
        move_right();
    }
    // backward
    fill_rectangle_char(0, 0, 22, 22, "["); // horizontal stripe
    for (player_x = 1; player_x < MAP_SIZE - 2;)
    {
        move_backward();
    }
    // left
    fill_rectangle_char(0, 0, 22, 22, "\\"); // vertical stripe
    for (player_y = MAP_SIZE - 2; player_y > 1;)
    {
        move_left();
    }
}

void main()
{       
    print_string("Initialising...");
    start_attr_address = (unsigned char*)(ATTR_BUFF); // start of map attribute memory
    init_map();    
    // try out some fill_rectangle stuff
    fill_rectangle_char(0, 0, 24, 32, background_pattern1); // repeating background pattern
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
    copy_attr_buffer();
    fill_rectangle_char(0, 0, 24, 32, " "); // repeating background pattern
    fill_rectangle_attr(0, 0, 24, 32, 0, 7);
    copy_attr_buffer();
    do
    {        
        // loop around map
        loop_around_map();
    } while (1);
}