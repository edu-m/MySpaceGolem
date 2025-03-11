/* This api contains all the functions that need some IO operation
 *
 * by Andrea Malatesti and Simone Guercini
 * Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
 * plus a waiver of all other intellectual property. The goal of this work is to
 * be and remain completely in the public domain forever, available for any use
 * whatsoever.
 */

// Each animation has 4 frames, from 0 to 3
#define ANIMATION_FRAMES 4

enum Input {NONE, UP, LEFT, RIGHT, DOWN, BUTTON_A, BUTTON_B, QUIT};
enum Input get_input(void);
int init_game(const char *title, int size, int sidebar);
void close_game(void);
void draw_ship(int x, int y);
void draw_base(int x, int y, int frame); 
void draw_mineral(int x, int y, int frame); 
void draw_explosion(int x, int y, int frame); 
void draw_space(void);
void draw_asteroid(int x, int y);
enum PathType {PATH_DOT, PATH_BIG, PATH_CROSS, PATH_LASER1, PATH_LASER2};
void draw_path(int x, int y, enum PathType type);
void draw_laser(int x, int y, int frame); 
void draw_arrow(int x, int y, int angle);
void draw_text(int x, int y, char* str);
void screen_refresh(void);
