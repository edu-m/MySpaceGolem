/* This is the implementation of the game logic without IO,
 * as the implementation of the functions in ./io_api.h will be in files named "*_io.c"
 *
 * by Andrea Malatesti and Simone Guercini
 * Released under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/)
 * plus a waiver of all other intellectual property. The goal of this work is to
 * be and remain completely in the public domain forever, available for any use
 * whatsoever.
 */
#include <stdint.h>
#include "./io_api.h"


// used to make the code more readable when handling positions
typedef struct {
  int x;
  int y;
} Position;

typedef struct {
  enum {WHOLE, BROKEN, TAKEN, IN_BASE} state;
  Position pos;
} Mineral;

// number of cells on a side of the the game grid
// (the grid is squared, all sides are equal)
#define SIZE 16
static int isAsteroid[SIZE*SIZE];
Mineral mineral;

typedef struct {
  int speedX;
  int speedY;
  Position pos;
  Position path[SIZE];
  int pathlen;
} Player;

enum AnimState
  {NO_ANIM,
   LASER_ANIM,
   MINERAL_ANIM,
   MOVE_ANIM,
   EXPLOSION_ANIM,
   BASE_ANIM};


static int abs(int a) {
  return a >= 0 ? a : -a;
}
static int max(int a, int b) {
  return a > b ? a : b;
}
static int min(int a, int b) {
  return a < b ? a : b;
}

static uint32_t _currentRand;
static void random_seed(uint32_t seed) {
  _currentRand = seed;
}
static unsigned random_gen() {
  _currentRand = 2891336453 * _currentRand + 123;
  return _currentRand >> 16;
}

static int pos_eq(Position a, Position b) {
  return a.x == b.x && a.y == b.y;
}

static void n2str(int n, char str[11]) {
  int i = 0;
  for(int k = 100000000; k; k /= 10) {
    int digit = (n / k) % 10;
    str[i] = digit + '0';
    i++;
  }
  str[9] = 0;
}

static int main_menu() {
  int pointingAt = 0;
  int step = 0;
  int ax = 0;
  char seedStr[32] = "seed:";
  int seed = 12345;
  n2str(seed, &seedStr[5]);
  char *options[] =
    {"play",
     seedStr,
     "tutorial",
     "quit"};
  while (1) {
    switch(get_input()) {
    case UP:
      if(step > 0) {
	seed = min(seed + step, 100000000);
	n2str(seed, &seedStr[5]);
      } else pointingAt = max(0, pointingAt-1);
      break;
    case DOWN:
      if(step > 0) {
	seed = max(0, seed - step);
	n2str(seed, &seedStr[5]);
      } else pointingAt = min(3, pointingAt+1);
      break;
    case RIGHT:
      if(step > 0) {
	ax++;
	step /= 10;
      }
      break;
    case LEFT:
      if(step > 0 && ax > -7) {
	ax--;
	step *= 10;
      }
      break;
    case BUTTON_A:
      switch(pointingAt) {
      case 0:
	random_seed(seed);
	return 0;
      case 1:
	step = ax = step == 0;
	break;
      case 3:return 1;
      }
      break;
    case BUTTON_B:
      if(step == 0)
	pointingAt = 3;
      else step = 0;
      break;
    case QUIT:
      return 1;
    default:
      break;
    }
    draw_space();
    for(int i=0; i<4; i++)
      draw_text(i==pointingAt, i+1, options[i]);
    if(step > 0)
      draw_arrow(13 + ax, pointingAt, 1);
    else draw_arrow(0, 1+pointingAt, 0);
    screen_refresh();
  }
}

static int game_over() {
  while (1) {
    switch(get_input()) {
    case NONE:
      break;
    case QUIT:
      return 1;
    default:
      return 0;
    break;
    }
    draw_space();
    draw_text(2, 2, "game over");
    draw_text(1, 4, "press any key");
    screen_refresh();
  }
  return 0;
}

static int pause_menu(void) {
  int pointingAt = 0;
  char *options[] =
    {"resume",
     "main menu",
     "quit"};
  while (1) {
    switch(get_input()) {
    case UP:
      pointingAt = max(0, pointingAt-1);
      break;
    case DOWN:
      pointingAt = min(2, pointingAt+1);
      break;
    case BUTTON_B:
      pointingAt = 2;
      break;
    case BUTTON_A:
      return pointingAt;
    case QUIT:
      return 2;
    default:
      break;
    }
    draw_space();
    for(int i=0; i<3; i++)
      draw_text(i==pointingAt, i+1, options[i]);
    draw_arrow(0, 1+pointingAt, 0);
    screen_refresh();
  }
  return 0;
}

int play_game(void);
int main(void) {
  if (!init_game("Space_GOLEM", SIZE, 8))
    return 1;
  // if any of main_menu or play_game returns 1, it means the game should close,
  // otherwise, keep running
  while(!(main_menu() || play_game()));
  close_game();
  return 0;
}

/* This function writes to path[0 ... len-1] the sequence of positions
 * of each point of a rasterized line starting from (x,y) and pointing
 * in the direction (dx,dy).
 * It uses the Bresenham's line algorithm.
 */

static void line_path(int x, int y, int dx, int dy, Position *path, int len) {
    int sx = dx >= 0 ? 1 : -1, sy = dy >= 0 ? 1 : -1;
    int adx = abs(dx), ady = abs(dy);
    dx = -adx; dy = ady;
    int error = dx + dy;
    path[0] = (Position){x, y};
    for (int i = 1; i < len; i++) {
        int error2 = error * 2;
        if (error2 > dx) {
            error += dx;
            if (y + sy < 0 || y + sy >= SIZE)
                sy = -sy;
            y += sy;
        }
        if (error2 < dy) {
            error += dy;
            if (x + sx < 0 || x + sx >= SIZE)
                sx = -sx;
            x += sx;
        }
        path[i] = (Position){x, y};
    }
}


//utility functions
Position spawn_in_rect(int offset_x, int offset_y, int w, int h) {
  return (Position) {offset_x + random_gen()%w,
		     offset_y + random_gen()%h};
}

/* return 0 if no collision,
 * 1 if collides with asteroid,
 * 2 if collides with whole mineral
 */
#if(0)
int check_collision(Position p) {
if(isAsteroid[p.x + p.y * SIZE])
    return 1;
  if(pos_eq(p, mineral.pos) && mineral.state == WHOLE)
    return 2;
  return 0;
}
#else
int check_collision(Position p){
	const int ast = isAsteroid[p.x + p.y * SIZE];
	return  ast * 1 + (1 - ast) * pos_eq(p, mineral.pos) && mineral.state == WHOLE * 2;
}
#endif
// how many cell separate the spawn area from the border
#define SPAWN_OFFSET 2
// height of the spawn area
#define SPAWN_HEIGHT 3
// number of asteroids
#define NUM_ASTEROIDS 8

/* returns a bool, telling if the program should terminate:
 * If true -> close everything
 * If false -> stay on the menu screen
 */
int play_game(){
 Position b=spawn_in_rect(SPAWN_OFFSET,SIZE-(SPAWN_OFFSET+SPAWN_HEIGHT),SIZE-SPAWN_OFFSET*2,SPAWN_HEIGHT);
 mineral=(Mineral){WHOLE,spawn_in_rect(SPAWN_OFFSET,SPAWN_OFFSET,SIZE-SPAWN_OFFSET*2,SPAWN_HEIGHT)};
 for(int i=0;i<SIZE*SIZE;i++) isAsteroid[i]=0;
 for(int i=0;i<NUM_ASTEROIDS;i++){
  Position a=spawn_in_rect(0,SPAWN_OFFSET+SPAWN_HEIGHT,SIZE,SIZE-SPAWN_OFFSET*2-SPAWN_HEIGHT*2);
  isAsteroid[a.y*SIZE+a.x]=1;
 }
 Player sh={.pos={SPAWN_OFFSET,SIZE-(SPAWN_OFFSET+SPAWN_HEIGHT)}};
 enum Input pi=NONE;
 enum AnimState as=NO_ANIM;
 int af=0,sp=0,am=0;
 Position ar={-1,-1};
 int an=-1;
 for(;;){
  enum Input act=(pi==NONE&&as==NO_ANIM)?BUTTON_A:get_input();
  if(act==QUIT)return 1;
  else if(act==BUTTON_B){int r=pause_menu(); if(r)return r-1;}
  else if(act!=NONE&&as==NO_ANIM){
   if(act==pi){
    switch(act){
     case UP:sh.speedY--;break;
     case DOWN:sh.speedY++;break;
     case LEFT:sh.speedX--;break;
     case RIGHT:sh.speedX++;break;
     default:break;
    }
    int x=sh.pos.x+sh.speedX;
    if(x<0||x>=SIZE)sh.speedX=-sh.speedX;
    int y=sh.pos.y+sh.speedY;
    if(y<0||y>=SIZE)sh.speedY=-sh.speedY;
    as=(act==BUTTON_A&&am)?LASER_ANIM:MOVE_ANIM;
    pi=NONE;
   } else {
    int dx=0,dy=0;
    switch(act){
     case UP:dy=-1,an=3;break;
     case DOWN:dy=1,an=1;break;
     case LEFT:dx=-1,an=2;break;
     case RIGHT:dx=1,an=0;break;
     default:break;
    }
    line_path(sh.pos.x,sh.pos.y,sh.speedX+dx,sh.speedY+dy,sh.path,SIZE);
    sp=min(max(abs(sh.speedX+dx),abs(sh.speedY+dy)),SIZE-1);
    am=0;
    for(sh.pathlen=1;sh.pathlen<SIZE;sh.pathlen++){
     int r=check_collision(sh.path[sh.pathlen]);
     if(r>0){am=(act==BUTTON_A&&r==2);break;}
    }
    if(act==BUTTON_A){ar=sh.path[sp];an=-1;}
    pi=act;
   }
  }
  switch(as){
   case LASER_ANIM:
    if(af==ANIMATION_FRAMES){as=MINERAL_ANIM;af=0;}
    break;
   case MINERAL_ANIM:
    if(af==ANIMATION_FRAMES){mineral.state=BROKEN;as=MOVE_ANIM;af=0;}
    break;
   case MOVE_ANIM:
    sh.pos=sh.path[af];
    if(check_collision(sh.pos)){as=EXPLOSION_ANIM;af=0;break;}
    if(pos_eq(sh.pos,mineral.pos)&&mineral.state==BROKEN)mineral.state=TAKEN;
    if(pos_eq(sh.pos,b)&&mineral.state==TAKEN)mineral.state=IN_BASE;
    if(af==sp){as=(mineral.state==IN_BASE)?BASE_ANIM:NO_ANIM;af=0;}
    break;
   case EXPLOSION_ANIM:
    if(af==ANIMATION_FRAMES)return game_over();
    break;
   case BASE_ANIM:
    if(af==ANIMATION_FRAMES){mineral.state=WHOLE;as=NO_ANIM;af=0;}
    break;
   case NO_ANIM:break;
  }
  draw_space();
  if(mineral.state==BROKEN)
   draw_mineral(mineral.pos.x,mineral.pos.y,3);
  else if(mineral.state==WHOLE)
   draw_mineral(mineral.pos.x,mineral.pos.y,(as==MINERAL_ANIM)?af:0);
  for(int i=0;i<SIZE*SIZE;i++) if(isAsteroid[i]) draw_asteroid(i%SIZE,i/SIZE);
  if(as==LASER_ANIM)
   for(int i=sh.pathlen-1;i>0;i--) draw_laser(sh.path[i].x,sh.path[i].y,af);
  else if(as==NO_ANIM&&pi!=NONE){
   if((sh.speedX||sh.speedY)&&an>=0)draw_arrow(ar.x,ar.y,an);
   if(sp)
    for(int i=sh.pathlen-1;i>0;i--){
     int X=sh.path[i].x,Y=sh.path[i].y;
     if(an>=0&&pos_eq(sh.path[i],ar))continue;
     else if(i==sp)draw_path(X,Y,PATH_CROSS);
     else if(am)draw_path(X,Y,(i%sp)?PATH_LASER1:PATH_LASER2);
     else draw_path(X,Y,(i%sp)?PATH_DOT:PATH_BIG);
    }
  }
  draw_base(b.x,b.y,(as==BASE_ANIM)?af:0);
  if(as!=EXPLOSION_ANIM)draw_ship(sh.pos.x,sh.pos.y);
  else draw_explosion(sh.pos.x,sh.pos.y,af);
  if(as!=NO_ANIM)af++;
  screen_refresh();
 }
}

