/* This is the implementation of the game IO using SDL2,
 * while the game logic resides in main.c
 *
 * by Andrea Malatesti and Simone Guercini
 * Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
 * plus a waiver of all other intellectual property. The goal of this work is to
 * be and remain completely in the public domain forever, available for any use
 * whatsoever.
 */


#include <SDL2/SDL.h>
#include "./io_api.h"

static SDL_Window *myWin; 
static SDL_Renderer *myRen;
// if FONT_SIZE is x, then a character has the size of x*x pixels
#define FONT_SIZE 10
static SDL_Texture* font;
static SDL_Texture* tileSet;

// Length of a game tile in the grid
#define LEN 16
// If the zoom is x, each pixel will be a square x*x
#define ZOOM 2

static SDL_Texture* load_texture(char* path) {
  if(!path)
    return NULL;

  SDL_Surface* bitMapFile = SDL_LoadBMP(path);
  if(bitMapFile == NULL)
    return NULL;

  // make the black color transparent
  SDL_SetColorKey(bitMapFile, SDL_TRUE, 0);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(myRen, bitMapFile);
  SDL_FreeSurface(bitMapFile);
  return texture;
}

static int gameSize;
int init_game(char const *title, int size, int sidebar) {
  if(SDL_Init(SDL_INIT_VIDEO)) 
    return 0;
  
  myWin = SDL_CreateWindow
    (title,
     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
     (size + sidebar) * LEN * ZOOM, size * LEN * ZOOM,
     SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);

  
  if(myWin == NULL) 
    return 0;

  gameSize = size;
  
  myRen = SDL_CreateRenderer(myWin, -1, SDL_RENDERER_ACCELERATED);
  
  if(myRen == NULL) 
    return 0;

  SDL_RenderSetLogicalSize(myRen, (size + sidebar) * LEN, size * LEN);
  
  //SDL_RenderSetIntegerScale(myRen, 1);
  tileSet = load_texture("./sprites/tlset.bmp");
  if(!tileSet)
    return 0;

  font = load_texture("./sprites/font.bmp");
  if(!font)
    return 0;
  
  return 1;
}

void close_game(void) {
  char const* error = SDL_GetError();
  if(error[0]) fputs(error, stderr);
  
  if (myRen != NULL) {
    SDL_DestroyRenderer(myRen);
    myRen = NULL;
  }
  if (myWin != NULL) {
    SDL_DestroyWindow(myWin);
    myWin = NULL;
  }
  if (tileSet != NULL) {
    SDL_DestroyTexture(tileSet);
    tileSet = NULL;
  }
  if (font != NULL) {
    SDL_DestroyTexture(font);
    font = NULL;
  }
}


void draw_ship(int x, int y) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {0, 0, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
}

void draw_base(int x, int y, int frame) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {frame*LEN, 64, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
}

void draw_mineral(int x, int y, int frame) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {frame*LEN, 16, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
} 

void draw_laser(int x, int y, int frame) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {frame*LEN, 32, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
} 

void draw_explosion(int x, int y, int frame) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {frame*LEN, 48, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
} 

void draw_space(void) {
  SDL_SetRenderDrawColor(myRen, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(myRen);
  for(int i=1; i<16; i++) {
    int x = (i * 533) % (gameSize*LEN-LEN/2);
    int y = (i * 313) % (gameSize*LEN-LEN/2);
    int index = i%4;
    SDL_RenderCopy
      (myRen, tileSet,
       &(SDL_Rect) {index*8, 88, LEN/2, LEN/2},
       &(SDL_Rect) {x, y, LEN/2, LEN/2});
  }
  SDL_SetRenderDrawColor(myRen, 0, 0xff, 0xff, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawRect(myRen, &(SDL_Rect) {0, 0, 16*LEN, 16*LEN});
}

void draw_asteroid(int x, int y) {
  int i = x+y*2;
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {LEN*(1+(i%3)), 0, LEN, LEN},
     &(SDL_Rect) {x*LEN, y*LEN, LEN, LEN});
}

void screen_refresh(void) {
  SDL_RenderPresent(myRen);
  SDL_Delay(50);
}

enum Input get_input(void) {
  enum Input action = NONE;
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      action = QUIT;
      break;
				
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_w:
      case SDLK_k:
	action = UP;
	break;
      case SDLK_a:
      case SDLK_h:
	action = LEFT;
	break;
      case SDLK_s:
      case SDLK_j:
	action = DOWN;
	break;
      case SDLK_d:
      case SDLK_l:
	action = RIGHT;
	break;
      case SDLK_RETURN: action = BUTTON_A; break;
      case SDLK_SPACE: action = BUTTON_B; break;
      case SDLK_ESCAPE: action = QUIT; break;
      }
      break;
    }
  }
  return action;
}

void draw_path(int x, int y, enum PathType type) {
  SDL_RenderCopy
    (myRen, tileSet,
     &(SDL_Rect) {(LEN/2)*type, 80, LEN/2, LEN/2},
     &(SDL_Rect) {x*LEN+LEN/4, y*LEN+LEN/4, LEN/2, LEN/2});
}

void draw_arrow(int x, int y, int angle) {
  SDL_RenderCopyEx
    (myRen, tileSet,
     &(SDL_Rect) {32, 88, LEN/2, LEN/2}, 
     &(SDL_Rect) {x*LEN+LEN/4, y*LEN+LEN/4, LEN/2, LEN/2},
     angle * 90,
     NULL,
     SDL_FLIP_NONE);
}

void draw_text(int x, int y, char* str) {
  for(int i=0; str[i]; i++) {
    SDL_RenderCopy
      (myRen, font,
       &(SDL_Rect) {(str[i] - ' ') * FONT_SIZE, 0, FONT_SIZE, FONT_SIZE},
       &(SDL_Rect) {(x+i)*LEN+(LEN-FONT_SIZE)/2, y*LEN+(LEN-FONT_SIZE)/2, FONT_SIZE, FONT_SIZE});
  }
}
