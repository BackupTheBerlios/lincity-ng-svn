#ifndef _POWER_H
#define _POWER_H

#undef DEBUG_POWER

struct grid_struct {
  int power_lines;
  long total_power; 
  long avail_power; /* Capacity available */
  long max_power;
  long demand;
  short powered;
};

typedef struct grid_struct Grid;

/* public */
#define MAX_GRIDS 128 // How many grids in the array, not how many to allocate
extern Grid * grid[MAX_GRIDS];

void map_power_grid(void);
int get_power (int x, int y, int power, int block_industry);
void do_windmill(int x, int y);
void do_power_substation(int x, int y);
void do_power_source(int x, int y);
void do_power_source_coal(int x, int y);
void do_power_line(int x, int y);

/* intended private */
void recurse_power_grid (int startx, int starty);
int check_grid(int x, int y, int xi, int yi);
void project_power(int x, int y);

/* #ifdef POWER_LINE_CAPACITY
#undef POWER_LINE_CAPACITY
#endif */

#define POWER_LINE_LOSS 1 /* one KW */

#define WEST 1
#define NORTH 2
#define EAST 3
#define SOUTH 4

#define XY_IS_GRID(x,y) \
((MP_GROUP(x,y) == GROUP_COAL_POWER) ||\
 (MP_GROUP(x,y) == GROUP_WINDMILL) || \
 (MP_GROUP(x,y) == GROUP_POWER_LINE) || \
 (MP_GROUP(x,y) == GROUP_SOLAR_POWER) || \
 (MP_GROUP(x,y) == GROUP_SUBSTATION))

#define XY_IS_TRANSPORT(x,y) \
((MP_GROUP(x,y) == GROUP_TRACK) || \
 (MP_GROUP(x,y) == GROUP_ROAD) || \
 (MP_GROUP(x,y) == GROUP_RAIL) || \
 (MP_GROUP(x,y) == GROUP_POWER_LINE))

#define XY_IS_WATER(x,y) (MP_GROUP(x,y) == GROUP_WATER)

#define IS_POWER_SOURCE(x,y) \
((MP_GROUP(x,y) == GROUP_COAL_POWER) || \
 (MP_GROUP(x,y) == GROUP_SOLAR_POWER) || \
 (MP_GROUP(x,y) == GROUP_WINDMILL))

#define IS_POWER_LINE(x,y) (MP_GROUP(x,y) == GROUP_POWER_LINE)

#define IS_OLD_WINDMILL(x,y) \
((MP_GROUP(x,y) == GROUP_WINDMILL) && \
 (MP_TYPE(x,y) != CST_USED) && \
 (MP_INFO(x,y).int_2 < MODERN_WINDMILL_TECH))

#define GRID_CURRENT(x,y) (MP_INFO(x,y).int_7 == grid_inc)

/*** JOBS ***/
#define SOLAR_POWER_JOBS 50

#endif
