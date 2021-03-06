/* ---------------------------------------------------------------------- *
 * solar_power.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */

#include "modules.h"
#include "../power.h"
#include "solar_power.h"

/*** Solar Power ***/

/*
  int_1 unused
  int_2 is the tech level when it was built.
  int_3 is the rated output
  int_4 unused
  int_5 is the current output
  int_6 is the grid it's on
  int_7 is a grid mapping timestamp
*/

void
do_power_source (int x, int y)
{
    /* Al1: fix for stupid bug i introduced before 1.1.1 => int1 and int3 had wrong valued as if tech = 0 ! */
    MP_INFO(x,y).int_1 = (int)(POWERS_SOLAR_OUTPUT
            + (((double) MP_INFO(x,y).int_2 * POWERS_SOLAR_OUTPUT) / MAX_TECH_LEVEL));
    MP_INFO(x,y).int_3 = MP_INFO(x,y).int_1;
    /* end of fix */

    if (get_jobs(x, y, SOLAR_POWER_JOBS)) {
	MP_INFO(x,y).int_5 = MP_INFO(x,y).int_3;
	grid[MP_INFO(x,y).int_6]->avail_power += MP_INFO(x,y).int_3;
    } else {
	MP_INFO(x,y).int_5 = 0;
    }
}


void
mps_solar_power (int x, int y)
{
  int i = 0;

  char s[12];

  mps_store_title(i++,_("Solar"));
  mps_store_title(i++,_("Power Station"));
  i++;

  format_power (s, sizeof(s), MP_INFO(x,y).int_3);
  mps_store_title(i++,_("Max Output"));
  mps_store_title(i++,s);
  i++;

  format_power (s, sizeof(s), MP_INFO(x,y).int_5);
  mps_store_title(i++,_("Current Output"));
  mps_store_title(i++,s);
  i++;

  mps_store_sfp(i++,_("Tech"),
		MP_INFO(x,y).int_2 * 100.0 / MAX_TECH_LEVEL);  
  mps_store_sd(i++,_("Grid ID"), MP_INFO(x,y).int_6);
}
