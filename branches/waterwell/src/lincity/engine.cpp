/* ---------------------------------------------------------------------- *
 * engine.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include "common.h"
#include "lctypes.h"
#include "lin-city.h"
#include "engine.h"
#include "engglobs.h"
//#include "cliglobs.h"
#include "simulate.h"
#include "lcintl.h"
#include "power.h"
//#include "mouse.h"
//#include "module_buttons.h"
#include "gui_interface/pbar_interface.h"
#include "stats.h"
//#include "screen.h"
#include "gui_interface/dialbox_interface.h"
#include "gui_interface/mps.h"
#include "gui_interface/screen_interface.h"
#include "gui_interface/shared_globals.h"

extern int selected_type_cost;
static void bulldoze_mappoint (short fill, int x, int y);
int is_real_river (int x, int y);

int last_warning_message_group = 0;

void
fire_area (int xx, int yy)
{
    /* this happens when a rocket crashes or on random_fire. */
    int x = xx;
    int y = yy;
    int size;
    if (MP_GROUP(x,y) == GROUP_WATER || MP_GROUP(x,y) == GROUP_FIRE)
        return;
    if (MP_TYPE(x,y) == CST_USED) {
        x = MP_INFO(xx,yy).int_1;
        y = MP_INFO(xx,yy).int_2;
    }
    size = MP_SIZE(x,y);

    /* Destroy the content of the building to prevent special management
     * when bulldozed.
     */

    /* Kill 'only' half of the people (bulldoze item put them in people_pool)
     * lincity NG 1.1 and previous killed all the people!
     */
    if ((MP_INFO(x,y).flags & FLAG_FIRE_COVER) !=0)
        MP_INFO(x,y).population = MP_INFO(x,y).population / 2;
    else
        MP_INFO(x,y).population = 0;
    MP_INFO(x,y).flags = 0;
    MP_INFO(x,y).int_1 = 0;
    MP_INFO(x,y).int_2 = 0;
    MP_INFO(x,y).int_3 = 0;
    MP_INFO(x,y).int_4 = 0;
    MP_INFO(x,y).int_5 = 0;
    MP_INFO(x,y).int_6 = 0;
    MP_INFO(x,y).int_7 = 0;

    /* Correctly remove buildings (substations...) and adjust count,
     * but don't count bulldoze cost
     * */
    adjust_money(+main_groups[MP_GROUP(x,y)].bul_cost);
    bulldoze_item(x, y);

    /* put fire */
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++) {
            bulldoze_mappoint (CST_FIRE_1, x + i, y + j);
            MP_GROUP(x + i, y + j) = GROUP_FIRE;
        }


    /* AL1: is it necessary ? It is the only place in lincity/. with such a call
     *  all other are in lincity-ng/.
     */
    refresh_main_screen ();

  /* 
    // update transport or we get stuff put in
    // the area from connected tracks etc.
    // FIXME: AL1: NG 1.1: do the right thing and/or remove this comment
  */
}

int 
adjust_money(int value)
{
    total_money += value;
    print_total_money();
    mps_update();
    update_pbar (PMONEY, total_money, 0);
    refresh_pbars(); /* This could be more specific */
    return total_money;
}

int
no_credit_build (int selected_group)
{
  if (total_money >= 0)
    return (0);

#ifdef GROUP_SOLAR_POWER_NO_CREDIT
  if (selected_group == GROUP_SOLAR_POWER) {
    return (1);
  }
#endif
#ifdef GROUP_UNIVERSITY_NO_CREDIT
  if (selected_group == GROUP_UNIVERSITY) {
    return (1);
  }
#endif
#ifdef GROUP_PARKLAND_NO_CREDIT
  if (selected_group == GROUP_PARKLAND) {
    return (1);
  }
#endif
#ifdef GROUP_RECYCLE_NO_CREDIT
  if (selected_group == GROUP_RECYCLE) {
    return (1);
  }
#endif
#ifdef GROUP_ROCKET
  if (selected_group == GROUP_ROCKET) {
    return (1);
  }
#endif

  if (main_groups[selected_group].no_credit == TRUE ) {
    return (1);
  }
  return (0);
}

void 
no_credit_build_msg_ng (int selected_group)
{
  if (last_warning_message_group == selected_group)
        return;
  last_warning_message_group = selected_group;

#ifdef GROUP_SOLAR_POWER_NO_CREDIT
  if (selected_group == GROUP_SOLAR_POWER) {
    ok_dial_box ("no-credit-solar-power.mes", BAD, 0L);
    return;
  }
#endif
#ifdef GROUP_UNIVERSITY_NO_CREDIT
  if (selected_group == GROUP_UNIVERSITY) {
    ok_dial_box ("no-credit-university.mes", BAD, 0L);
    return;
  }
#endif
#ifdef GROUP_PARKLAND_NO_CREDIT
  if (selected_group == GROUP_PARKLAND) {
    ok_dial_box ("no-credit-parkland.mes", BAD, 0L);
    return;
  }
#endif
#ifdef GROUP_RECYCLE_NO_CREDIT
  if (selected_group == GROUP_RECYCLE) {
    ok_dial_box ("no-credit-recycle.mes", BAD, 0L);
    return;
  }
#endif
#ifdef GROUP_ROCKET
  if (selected_group == GROUP_ROCKET) {
    ok_dial_box ("no-credit-rocket.mes", BAD, 0L);
    return;
  }
#endif
  return;
}

int 
place_item (int x, int y, short type)
{
    int i,j;
    int prev_tip = 0;
    int group;
    int size;

    group = get_group_of_type(type);
    if (group < 0) {
#ifdef DEBUG
        fprintf(stderr,"Error: group does not exist %i\n", group);
#endif
        ok_dial_box ("warning.mes", BAD,
                _("ERROR: group does not exist. This should not happen! Please consider filling a bug report to lincity-ng team, with the saved game and what you did :-) "));
        return -1000;
    }

    size = main_groups[group].size;

    /* You can't build because credit not available. */
    if (no_credit_build (group) != 0) {
        no_credit_build_msg_ng (group);
	return -1;
    }

    switch (group) {
    case GROUP_ORGANIC_FARM:
	MP_INFO(x,y).int_1 = tech_level;
        break;
    case GROUP_TRACK:
    case GROUP_ROAD:
    case GROUP_RAIL:
	MP_INFO(x,y).flags |= FLAG_IS_TRANSPORT;
        break;
    case GROUP_PORT:
	if (is_real_river (x + 4, y) != 1 
	        || is_real_river (x + 4, y + 1) != 1
	        || is_real_river (x + 4, y + 2) != 1 
	        || is_real_river (x + 4, y + 3) != 1) {
            if (last_warning_message_group != group)
                ok_dial_box ("warning.mes", BAD,
                    _("Port must be connected to river all along right side."));
            last_warning_message_group = group;
            return -2;
        }
        break;
    case GROUP_SUBSTATION:
        if (add_a_substation (x, y) == 0) {
            /* Not enough slots in the substation array */
            if (last_warning_message_group != group)
                ok_dial_box ("warning.mes", BAD,
                    _("Too many substations + windmills. You cannot build one more"));
            last_warning_message_group = group;
	    return -3;
        }
        break;
    case GROUP_WINDMILL:
	if (add_a_substation (x, y) == 0) {
            /* Not enough slots in the substation array */
            if (last_warning_message_group != group)
                ok_dial_box ("warning.mes", BAD,
                    _("Too many substations + windmills. You cannot build one more"));
            last_warning_message_group = group;
	    return -3;
        }
        MP_INFO(x,y).int_2 = tech_level;
        MP_INFO(x,y).int_1 = (int)(WINDMILL_POWER
	        + (((double) MP_INFO(x,y).int_2 * WINDMILL_POWER) / MAX_TECH_LEVEL));
        /* Make sure that the correct windmill graphic shows up */
	if (tech_level > MODERN_WINDMILL_TECH)
	    type = CST_WINDMILL_1_R;
	else 
	    type = CST_WINDMILL_1_W;
        break;
    case (GROUP_COAL_POWER):
        MP_INFO(x,y).int_4 = tech_level;
        MP_INFO(x,y).int_1 = (int)(POWERS_COAL_OUTPUT
	        + (((double) MP_INFO(x,y).int_4 * POWERS_COAL_OUTPUT)
	                / MAX_TECH_LEVEL));
        break;
    case (GROUP_SOLAR_POWER):
 	MP_INFO(x,y).int_2 = tech_level;
        MP_INFO(x,y).int_1 = (int)(POWERS_SOLAR_OUTPUT
	        + (((double) MP_INFO(x,y).int_2 * POWERS_SOLAR_OUTPUT)
	                / MAX_TECH_LEVEL)); /* like other power sources */
        MP_INFO(x,y).int_3 = MP_INFO(x,y).int_1; /* Int_3 is kept for compatibility */
        break;
    case GROUP_COMMUNE:
	numof_communes++;
        break;
    case GROUP_MARKET:
	/* Test for enough slots in the market array */
	if (add_a_market (x, y) == 0) {
            if (last_warning_message_group != group)
                ok_dial_box ("warning.mes", BAD,
                        _("Too many markets. You cannot build one more"));
            last_warning_message_group = group;
	    return -4;
        }
	MP_INFO(x,y).flags += (FLAG_MB_FOOD | FLAG_MB_JOBS
			       | FLAG_MB_COAL | FLAG_MB_ORE | FLAG_MB_STEEL
			       | FLAG_MB_GOODS | FLAG_MS_FOOD | FLAG_MS_JOBS
			       | FLAG_MS_COAL | FLAG_MS_GOODS | FLAG_MS_ORE
			       | FLAG_MS_STEEL);
        break;
    case GROUP_RECYCLE:
        MP_INFO(x,y).int_4 = tech_level;
        break;
    case GROUP_TIP:
	/* Don't build a tip if there has already been one.  If we succeed,
	   mark the spot permanently by "doubling" the ore reserve */
	prev_tip = 0;
	for (i=0; i < size; i++)
	    for (j=0; j < size; j++)
		if (MP_INFO(x+i,y+j).ore_reserve > ORE_RESERVE) {
		    prev_tip = 1;
		    break;
		}
	if (prev_tip) {
	    ok_dial_box ("warning.mes", BAD,
                    _("You can't build a tip here: this area was once a landfill"));
	    return -5;
	} else {
	    for (i=0; i < size; i++)
		for (j=0; j < size; j++)
		    MP_INFO(x+i,y+j).ore_reserve = ORE_RESERVE * 2;
	}
        break;
    case GROUP_OREMINE:
    {
	/* Don't allow new mines on old mines or old tips */
	/* GCS: mines over old mines is OK if there is enough remaining 
	        ore, as is the case when there is partial overlap. */
	int total_ore = 0;
	prev_tip = 0;
	for (i=0;i<size;i++) {
	    for (j=0;j<size;j++) {
		total_ore += MP_INFO(x+i,y+j).ore_reserve;
		if (MP_INFO(x+i,y+j).ore_reserve > ORE_RESERVE) {
		    prev_tip = 1;
		    break;
		}
	    }
	}
	if (prev_tip) {
	    ok_dial_box ("warning.mes", BAD,
                    _("You can't build a mine here: This area was once a landfill"));
	    return -6;
	}
	if (total_ore < MIN_ORE_RESERVE_FOR_MINE) {
	    ok_dial_box("warning.mes", BAD,
                    _("You can't build a mine here: there is no ore left at this site"));
	    return -7;
	}
        break;
    } 
    case GROUP_WATERWELL:
	numof_waterwell++;
        break;
    case GROUP_PARKLAND:
        if (use_waterwell)
            if (!HAS_UGWATER(x,y)) {
                ok_dial_box("warning.mes", BAD,
                        _("You can't build a park here: it is a desert, parks need water"));
                return -8;
            }

    } /* end case */
    last_warning_message_group = 0;

    /* Store last_built for refund on "mistakes" */
    last_built_x = x;
    last_built_y = y;

    set_mappoint (x, y, type);

    if (group == GROUP_RIVER)
	connect_rivers ();

    connect_transport (x-2,y-2,x+size+1,y+size+1);

    adjust_money(-selected_module_cost);
    map_power_grid();
    return 0;
}

int 
bulldoze_item (int x, int y)
{
    int g, size;

    if (MP_TYPE(x,y) == CST_USED) {
	/* This is considered "improper" input.  Silently ignore. */
#ifdef DEBUG
        fprintf(stderr," try to improperly bulldoze_item CST_USED\n");
#endif
	return -1;
    }

    size = MP_SIZE(x,y);
    g = MP_GROUP(x,y);
    people_pool += MP_INFO(x,y).population;

    if (g == GROUP_DESERT) {
	/* Nothing to do. */
	return -1;
    }
    else if (g == GROUP_SHANTY) {
	remove_a_shanty(x, y);
	adjust_money(-GROUP_SHANTY_BUL_COST);
        numof_shanties--;
    }
    else if (g == GROUP_FIRE) {
	if (MP_INFO(x,y).int_2 >= FIRE_LENGTH)
	    return -1;  /* Can't bulldoze ? */
	MP_INFO(x,y).int_2 = FIRE_LENGTH + 1;
	MP_TYPE(x,y) = CST_FIRE_DONE1;
	MP_GROUP(x,y) = GROUP_BURNT;
	adjust_money(-GROUP_BURNT_BUL_COST);
    }
    else {
	adjust_money(-main_groups[g].bul_cost);
         
        if (g == GROUP_COMMUNE)
             numof_communes--;

        if (g == GROUP_MARKET)
            remove_a_market (x, y);
 
        if (g == GROUP_SUBSTATION || g == GROUP_WINDMILL)
             remove_a_substation (x, y);
 
	if (g == GROUP_OREMINE)	{
	    int i, j;
	    for (j = 0; j < 4; j++)
		for (i = 0; i < 4; i++)
		    if (MP_INFO(x + i,y + j).ore_reserve < ORE_RESERVE / 2)
			do_bulldoze_area (CST_WATER, x + i, y + j);
	} else {
            /* keep compatibility for saving pre_waterwell loaded game */
            if (use_waterwell)
                do_bulldoze_area (CST_DESERT, x, y);
            else
                do_bulldoze_area (CST_GREEN, x, y);
        }
    }

    /* Tell mps about it, in case its selected */
    mps_update();
    return size;  /* No longer used... */
}

void
do_bulldoze_area (short fill, int xx, int yy)
{
  int size, x, y;
  if (MP_TYPE(xx,yy) == CST_USED)
    {
      x = MP_INFO(xx,yy).int_1;
      y = MP_INFO(xx,yy).int_2;
    }
  else
    {
      x = xx;
      y = yy;
    }
  size = MP_SIZE(x,y);
  for (int i = 0; i < size; i++)
      for (int j = 0; j < size; j++)  
          bulldoze_mappoint (fill, x+i, y+j);
}

static void 
bulldoze_mappoint (short fill, int x, int y)
{
    /* bulldoze preserve underground resources */
    MP_TYPE(x,y) = fill;
    MP_GROUP(x,y) = get_group_of_type(fill);
    if (MP_GROUP(x,y) < 0)
        MP_GROUP(x,y) = GROUP_BARE;
    MP_INFO(x,y).population = 0;
    MP_INFO(x,y).flags &= FLAG_HAS_UNDERGROUND_WATER;
    MP_INFO(x,y).int_1 = 0;
    MP_INFO(x,y).int_2 = 0;
    MP_INFO(x,y).int_3 = 0;
    MP_INFO(x,y).int_4 = 0;
    MP_INFO(x,y).int_5 = 0;
    MP_INFO(x,y).int_6 = 0;
    MP_INFO(x,y).int_7 = 0;
}

/** Mappoint array shuffles mappoint in order to stop linear simulation effects */

/** this is called on startup */
void init_mappoint_array (void)
{
    int x;
    for (x = 0; x < WORLD_SIDE_LEN; x++) {
	mappoint_array_x[x] = x;
	mappoint_array_y[x] = x;
    }
}

/** this is called at the beginning of every frame */
void shuffle_mappoint_array (void)
{
  int i, x, a;
  for (i = 0; i < SHUFFLE_MAPPOINT_COUNT; i++)
    {
      x = rand () % WORLD_SIDE_LEN;
      a = mappoint_array_x[i];
      mappoint_array_x[i] = mappoint_array_x[x];
      mappoint_array_x[x] = a;
      x = rand () % WORLD_SIDE_LEN;
      a = mappoint_array_y[i];
      mappoint_array_y[i] = mappoint_array_y[x];
      mappoint_array_y[x] = a;
    }
}

void
do_pollution ()
{
  int x, p;
  int* pol = &map.pollution[0][0];

  /* Kill pollution from top edge of map */
  do {
    if (*pol > 0)
      *pol /= POL_DIV;
  } while (++pol < &map.pollution[1][0]);


  x= 1;
  do
    {
      /* Kill some pollution from left edge of map */
      if (*pol++ > 0)
        *(pol-1) /= POL_DIV;
      do {
        if (*pol > 10) {
	  p = *pol / 16;
	  *pol -= p;
	  switch ( rand() % 11)
	    {         /* prevailing wind is *from* SW */
	    case 0:
	    case 1: /* up */
	    case 2:
	      *(pol - 1) += p;
	      break;
	    case 3:
	    case 4: /* right */
	    case 5:
	      *(pol + WORLD_SIDE_LEN) += p;
	      break;
	    case 6: /* down */
	    case 7:
	      *(pol + 1) += p;
	      break;
	    case 8: /* left */
	    case 9:
	      *(pol - WORLD_SIDE_LEN) += p;
	      break;
	    case 10:
	      *pol += p- 2;
	      break;
	    }
	}
      } while (++pol < &map.pollution[x][WORLD_SIDE_LEN-1]);
      /* Kill some pollution from right edge of map */
      if (*pol > 0)
        *pol /= POL_DIV;
      ++x;
    }
  while (++pol < &map.pollution[WORLD_SIDE_LEN-1][0]);

  /* Kill pollution from bottom edge of map */
  do {
    if (*pol > 0)
      *pol /= POL_DIV;
  } while (++pol < &map.pollution[WORLD_SIDE_LEN][0]);
}

void
clear_fire_health_and_cricket_cover (void)
{
  int x, y, m;
  m = 0xffffffff - (FLAG_FIRE_COVER | FLAG_HEALTH_COVER
		    | FLAG_CRICKET_COVER| FLAG_WATERWELL_COVER);
  for (y = 0; y < WORLD_SIDE_LEN; y++)
    for (x = 0; x < WORLD_SIDE_LEN; x++)
      MP_INFO(x,y).flags &= m;
  /* Wow... chache misses or what! */
}

void
do_fire_health_and_cricket_cover (void)
{
  int x, y;
  for (y = 0; y < WORLD_SIDE_LEN; y++)
    for (x = 0; x < WORLD_SIDE_LEN; x++)
      {
	/*  The next few lines need changing to test for */
	/*  the group if these areas are animated. */

	if (MP_GROUP(x,y) == GROUP_FIRESTATION)
	  do_fire_cover (x, y);
	else if (MP_TYPE(x,y) == CST_HEALTH)
	  do_health_cover (x, y);
	else if (MP_GROUP(x,y) == GROUP_CRICKET)
	  do_cricket_cover (x, y);
	else if (MP_GROUP(x,y) == GROUP_WATERWELL)
	  do_waterwell_cover (x, y);
      }
}

void
do_random_fire (int x, int y, int pwarning)	/* well random if x=y=-1 */
{
  int xx, yy;
  if (x == -1 && y == -1)
    {
      x = rand () % WORLD_SIDE_LEN;
      y = rand () % WORLD_SIDE_LEN;
    }
  else
    {
      if (x < 0 || x >= WORLD_SIDE_LEN || y < 0 || y >= WORLD_SIDE_LEN)
	return;
    }
  if (MP_TYPE(x,y) == CST_USED)
    {
      xx = MP_INFO(x,y).int_1;
      yy = MP_INFO(x,y).int_2;
      x = xx;
      y = yy;
    }
  xx = rand () % 100;
  if (xx >= (main_groups[MP_GROUP(x,y)].fire_chance))
    return;
  if ((MP_INFO(x,y).flags & FLAG_FIRE_COVER) != 0)
    return;
  if (pwarning)
    {
      if (MP_GROUP(x,y) == GROUP_POWER_LINE)
	ok_dial_box ("fire.mes", BAD, _("It's at a power line."));
      else if (MP_GROUP(x,y) == GROUP_SOLAR_POWER)
	ok_dial_box ("fire.mes", BAD, _("It's at a solar power station."));
      else if (MP_GROUP(x,y) == GROUP_SUBSTATION)
	ok_dial_box ("fire.mes", BAD, _("It's at a substation."));
      else if (MP_GROUP_IS_RESIDENCE(x,y))
	ok_dial_box ("fire.mes", BAD, _("It's at a residential area."));
      else if (MP_GROUP(x,y) == GROUP_ORGANIC_FARM)
	ok_dial_box ("fire.mes", BAD, _("It's at a farm."));
      else if (MP_GROUP(x,y) == GROUP_MARKET)
	ok_dial_box ("fire.mes", BAD, _("It's at a market."));
      else if (MP_GROUP(x,y) == GROUP_TRACK)
	ok_dial_box ("fire.mes", BAD, _("It's at a track."));
      else if (MP_GROUP(x,y) == GROUP_COALMINE)
	ok_dial_box ("fire.mes", BAD, _("It's at a coal mine."));
      else if (MP_GROUP(x,y) == GROUP_RAIL)
	ok_dial_box ("fire.mes", BAD, _("It's at a railway."));
      else if (MP_GROUP(x,y) == GROUP_COAL_POWER)
	ok_dial_box ("fire.mes", BAD, _("It's at a coal power station."));
      else if (MP_GROUP(x,y) == GROUP_ROAD)
	ok_dial_box ("fire.mes", BAD, _("It's at a road."));
      else if (MP_GROUP(x,y) == GROUP_INDUSTRY_L)
	ok_dial_box ("fire.mes", BAD, _("It's at light industry."));
      else if (MP_GROUP(x,y) == GROUP_UNIVERSITY)
	ok_dial_box ("fire.mes", BAD, _("It's at a university."));
      else if (MP_GROUP(x,y) == GROUP_COMMUNE)
	ok_dial_box ("fire.mes", BAD, _("It's at a commune."));
      else if (MP_GROUP(x,y) == GROUP_TIP)
	ok_dial_box ("fire.mes", BAD, _("It's at a tip."));
      else if (MP_GROUP(x,y) == GROUP_PORT)
	ok_dial_box ("fire.mes", BAD, _("It's at a port."));
      else if (MP_GROUP(x,y) == GROUP_INDUSTRY_H)
	ok_dial_box ("fire.mes", BAD, _("It's at a steel works."));
      else if (MP_GROUP(x,y) == GROUP_RECYCLE)
	ok_dial_box ("fire.mes", BAD, _("It's at a recycle centre."));
      else if (MP_GROUP(x,y) == GROUP_HEALTH)
	ok_dial_box ("fire.mes", BAD, _("It's at a health centre."));
      else if (MP_GROUP(x,y) == GROUP_ROCKET)
	ok_dial_box ("fire.mes", BAD, _("It's at a rocket site."));
      else if (MP_GROUP(x,y) == GROUP_WINDMILL)
	ok_dial_box ("fire.mes", BAD, _("It's at a windmill."));
      else if (MP_GROUP(x,y) == GROUP_SCHOOL)
	ok_dial_box ("fire.mes", BAD, _("It's at a school."));
      else if (MP_GROUP(x,y) == GROUP_BLACKSMITH)
	ok_dial_box ("fire.mes", BAD, _("It's at a blacksmith."));
      else if (MP_GROUP(x,y) == GROUP_MILL)
	ok_dial_box ("fire.mes", BAD, _("It's at a mill."));
      else if (MP_GROUP(x,y) == GROUP_POTTERY)
	ok_dial_box ("fire.mes", BAD, _("It's at a pottery."));
      else if (MP_GROUP(x,y) == GROUP_FIRESTATION)
	ok_dial_box ("fire.mes", BAD, _("It's at a fire station!!!."));
      else if (MP_GROUP(x,y) == GROUP_CRICKET)
	ok_dial_box ("fire.mes", BAD, _("It's at a sports field!!!."));
      else if (MP_GROUP(x,y) == GROUP_SHANTY)
	ok_dial_box ("fire.mes", BAD, _("It's at a shanty town."));
      else
	ok_dial_box ("fire.mes", BAD, _("UNKNOWN!"));
    }
  fire_area (x, y);
}

void do_daily_ecology ()
{
    for (int x = 0; x < WORLD_SIDE_LEN; x++)
        for (int y = 0; y < WORLD_SIDE_LEN; y++) {
            /* approximately 3 monthes needed to turn bulldoze area into green */
            if (MP_GROUP(x,y) == GROUP_DESERT && HAS_UGWATER(x,y) 
                    && rand() %300 == 1)
                do_bulldoze_area(CST_GREEN, x, y);
        }
}

/*
   // spiral round from startx,starty until we hit something of group group.
   // return the x y coords encoded as x+y*WORLD_SIDE_LEN
   // return -1 if we don't find one.
 */
int
spiral_find_group (int startx, int starty, int group)
{
  int i, j, x, y;
  x = startx;
  y = starty;
  /* let's just do a complete spiral for now, work out the bounds later */
  for (i = 1; i < (WORLD_SIDE_LEN + WORLD_SIDE_LEN); i++)
    {
      for (j = 0; j < i; j++)
	{
	  x--;
	  if (x > 0 && x < WORLD_SIDE_LEN && y > 0 && y < WORLD_SIDE_LEN)
	    if (MP_GROUP(x,y) == group)
	      return (x + y * WORLD_SIDE_LEN);
	}
      for (j = 0; j < i; j++)
	{
	  y--;
	  if (x > 0 && x < WORLD_SIDE_LEN && y > 0 && y < WORLD_SIDE_LEN)
	    if (MP_GROUP(x,y) == group)
	      return (x + y * WORLD_SIDE_LEN);
	}
      i++;
      for (j = 0; j < i; j++)
	{
	  x++;
	  if (x > 0 && x < WORLD_SIDE_LEN && y > 0 && y < WORLD_SIDE_LEN)
	    if (MP_GROUP(x,y) == group)
	      return (x + y * WORLD_SIDE_LEN);
	}
      for (j = 0; j < i; j++)
	{
	  y++;
	  if (x > 0 && x < WORLD_SIDE_LEN && y > 0 && y < WORLD_SIDE_LEN)
	    if (MP_GROUP(x,y) == group)
	      return (x + y * WORLD_SIDE_LEN);
	}
    }
  return (-1);
}

/*
   // spiral round from startx,starty until we hit a 2x2 space.
   // return the x y coords encoded as x+y*WORLD_SIDE_LEN
   // return -1 if we don't find one.
 */
int
spiral_find_2x2 (int startx, int starty)
{
  int i, j, x, y;
  x = startx;
  y = starty;
  /* let's just do a complete spiral for now, work out the bounds later */
  for (i = 1; i < (WORLD_SIDE_LEN + WORLD_SIDE_LEN); i++)
    {
      for (j = 0; j < i; j++)
	{
	  x--;
	  if (x > 1 && x < WORLD_SIDE_LEN - 2 && y > 1
	      && y < WORLD_SIDE_LEN - 2)
	    if (GROUP_IS_BARE(MP_GROUP(x,y))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y))
		&& GROUP_IS_BARE(MP_GROUP(x,y + 1))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y + 1)) )
	      return (x + y * WORLD_SIDE_LEN);
	}
      for (j = 0; j < i; j++)
	{
	  y--;
	  if (x > 1 && x < WORLD_SIDE_LEN - 2 && y > 1
	      && y < WORLD_SIDE_LEN - 2)
	    if (GROUP_IS_BARE(MP_GROUP(x,y))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y))
		&& GROUP_IS_BARE(MP_GROUP(x,y + 1))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y + 1)))
	      return (x + y * WORLD_SIDE_LEN);
	}
      i++;
      for (j = 0; j < i; j++)
	{
	  x++;
	  if (x > 1 && x < WORLD_SIDE_LEN - 2 && y > 1
	      && y < WORLD_SIDE_LEN - 2)
	    if (GROUP_IS_BARE(MP_GROUP(x,y))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y))
		&& GROUP_IS_BARE(MP_GROUP(x,y + 1))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y + 1)))
	      return (x + y * WORLD_SIDE_LEN);
	}
      for (j = 0; j < i; j++)
	{
	  y++;
	  if (x > 1 && x < WORLD_SIDE_LEN - 2 && y > 1
	      && y < WORLD_SIDE_LEN - 2)
	    if (GROUP_IS_BARE(MP_GROUP(x,y))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y))
		&& GROUP_IS_BARE(MP_GROUP(x,y + 1))
		&& GROUP_IS_BARE(MP_GROUP(x + 1,y + 1)))
	      return (x + y * WORLD_SIDE_LEN);
	}
    }
  return (-1);
}

void
connect_rivers (void)
{
  int x, y, count;
  count = 1;
  while (count > 0)
    {
      count = 0;
      for (y = 0; y < WORLD_SIDE_LEN; y++)
	for (x = 0; x < WORLD_SIDE_LEN; x++)
	  {
	    if (is_real_river (x, y) == 1)
	      {
		if (is_real_river (x - 1, y) == -1)
		  {
		    MP_INFO(x - 1,y).flags |= FLAG_IS_RIVER;
		    count++;
		  }
		if (is_real_river (x, y - 1) == -1)
		  {
		    MP_INFO(x,y - 1).flags |= FLAG_IS_RIVER;
		    count++;
		  }
		if (is_real_river (x + 1, y) == -1)
		  {
		    MP_INFO(x + 1,y).flags |= FLAG_IS_RIVER;
		    count++;
		  }
		if (is_real_river (x, y + 1) == -1)
		  {
		    MP_INFO(x,y + 1).flags |= FLAG_IS_RIVER;
		    count++;
		  }
	      }
	  }
    }
}

int
is_real_river (int x, int y)
{
  /* returns zero if not water at all or if out of bounds. */
  if (x < 0 || x >= WORLD_SIDE_LEN || y < 0 || y >= WORLD_SIDE_LEN)
    return (0);
  if (MP_GROUP(x,y) != GROUP_WATER)
    return (0);
  if (MP_INFO(x,y).flags & FLAG_IS_RIVER)
    return (1);
  return (-1);
}

/* Feature: coal survey should vary in price and accuracy with technology */
void 
do_coal_survey (void)
{
    if (coal_survey_done == 0) {
	adjust_money(-1000000);
	coal_survey_done = 1;
    }
}
