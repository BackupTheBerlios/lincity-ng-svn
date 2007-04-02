/* ---------------------------------------------------------------------- *
 * water.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */

#include "modules.h"
#include "waterwell.h"


void
mps_waterwell (int x, int y)
{
    int i = 0;

    const char * p;

    mps_store_title(i++,_("Water_well"));
    i++; /* blank line */

    mps_store_sd(i++, _("Pollution"), MP_POL(x,y));

    /* p = (MP_INFO(x,y).flags & FLAG_IS_RIVER) ? _("Yes") : _("No"); */
    p = _("Yes");
    mps_store_ss(i++,_("Drinkable"),p);

}

