/* ---------------------------------------------------------------------- *
 * recycle.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * (c) Corey Keasling, 2004
 * ---------------------------------------------------------------------- */


#include "recycle.h"
#include "../transport.h"

RecycleConstructionGroup recycleConstructionGroup(
    "Recyling Center",
    FALSE,                     /* need credit? */
    GROUP_RECYCLE,
    2,                         /* size */
    GROUP_RECYCLE_COLOUR,
    GROUP_RECYCLE_COST_MUL,
    GROUP_RECYCLE_BUL_COST,
    GROUP_RECYCLE_FIREC,
    GROUP_RECYCLE_COST,
    GROUP_RECYCLE_TECH
);

Construction *RecycleConstructionGroup::createConstruction(int x, int y, unsigned short type) {
    return new Recycle(x, y, type);
}


void Recycle::update()
{
    recycle_cost += RECYCLE_RUNNING_COST;
    //use some jobs for loading ore and steel to transport    
    if(commodityCount[STUFF_JOBS] >= JOBS_LOAD_ORE && commodityCount[STUFF_ORE] > 0)
    {    
        commodityCount[STUFF_JOBS] -= JOBS_LOAD_ORE;  
    }
    if(commodityCount[STUFF_STEEL] >= JOBS_LOAD_STEEL && commodityCount[STUFF_STEEL] > 0)
    {    
        commodityCount[STUFF_STEEL] -= JOBS_LOAD_STEEL;  
    }    

    // always recycle waste and make steel & ore if there are free capacities
    if (commodityCount[STUFF_WASTE] >= WASTE_RECYCLED
        && commodityCount[STUFF_KWH] >= KWH_RECYCLE_WASTE
        && commodityCount[STUFF_JOBS] >= RECYCLE_JOBS)
        {
            commodityCount[STUFF_JOBS] -= RECYCLE_JOBS;            
            commodityCount[STUFF_KWH] -= KWH_RECYCLE_WASTE;            
            commodityCount[STUFF_WASTE] -= WASTE_RECYCLED;
            workingdays++;            
            // rather loose ore than stop recycling the waste
            if (commodityCount[STUFF_ORE] + make_ore <= MAX_ORE_AT_RECYCLE)
            {            
                commodityCount[STUFF_ORE] += make_ore;
                ore_made += make_ore;
            }
            else
            {
                 ore_made += (MAX_ORE_AT_RECYCLE - commodityCount[STUFF_ORE]);
                 commodityCount[STUFF_ORE] = MAX_ORE_AT_RECYCLE;
            }      
            // rather loose steel than stop recycling the waste
            if (commodityCount[STUFF_STEEL] + make_steel <= MAX_STEEL_AT_RECYCLE)
            {            
                commodityCount[STUFF_STEEL] += make_steel;
            }
            else
            {
                 commodityCount[STUFF_STEEL] = MAX_STEEL_AT_RECYCLE;
            }           
        }
    // monthly update    
    if (total_time % 100 == 0)
    {
        busy = workingdays;
        workingdays = 0;
    }    
    // if we've still >90% waste in stock, burn some waste cleanly.     
    if (commodityCount[STUFF_WASTE] > (MAX_WASTE_AT_RECYCLE * 9 / 10))
    {
        commodityCount[STUFF_WASTE] -= BURN_WASTE_AT_RECYCLE;
    }
}

void Recycle::report()
{
    int i = 0;
    
    mps_store_sd(i++, constructionGroup->name,ID);
    i++;
    mps_store_sfp(i++, _("Tech"), tech * 100.0f / MAX_TECH_LEVEL);
    mps_store_sfp(i++, _("Efficiency"), eff);
    mps_store_sfp(i++, _("busy"), busy);
    i++;
    list_commodities(&i);
}

/** @file lincity/modules/recycle.cpp */

