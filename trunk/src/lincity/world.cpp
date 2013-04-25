/* ---------------------------------------------------------------------- *
 * world.cpp
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */

#include <stdlib.h>
#include <iostream>
#include "world.h"
#include "ConstructionCount.h"
#include "init_game.h"

World::World(int map_len)
{      
    //permutator = NULL;    
    //this->side_len = map_len;    
    maptile.resize(map_len * map_len);
    dirty = false;
    //std::cout << "created World len = " << len() << "²" << std::endl;
}

World::~World()
{        
    maptile.clear();  
}


void World::len(int new_len)
{       
    int len_sqr;
    int prev_len = len();    
    if (new_len < 50)
    {
        new_len = 50;
    }       
    if (dirty) {clear_game();}    
    this->side_len = new_len;    
    maptile.resize(new_len * new_len);    
    len_sqr = maptile.size();
          
    if (len_sqr != new_len * new_len)
    {
        std::cout << "failed to allocate World with side length = " << len() << std::endl;
        len(prev_len-25);
    }
}

MapTile* World::operator()(int x, int y)
{    
    return &(maptile[x + y * side_len]);
}

MapTile* World::operator()(int index)
{
    return &(maptile[index]);
}

bool World::is_inside(int x, int y)
{
    return (x >= 0 && y >= 0 && x < side_len && y < side_len);
}

bool World::is_inside(int index)
{
    return (index >= 0 && index < side_len * side_len);
}

bool World::is_border(int x, int y)
{
    return (x == 0 || y == 0 || x == side_len-1 || y == side_len -1);
}

bool World::is_border(int index)
{
    return (index%side_len == side_len -1 || index%side_len == 0 || index/side_len == side_len-1 || index/side_len == 0);
}

bool World::is_edge(int x, int y)
{
    return (x == 1 || y == 1 || x == side_len-2 || y == side_len -2);
}

bool World::is_visible(int x, int y)
{
    return (x > 0 && y > 0 && x < side_len-1 && y < side_len -1);
}

int World::map_x(MapTile * tile)
{
    return (tile-&maptile[0]) % side_len;
}

int World::map_y(MapTile * tile)
{
    return (tile-&maptile[0]) / side_len;
}

int World::map_index(MapTile * tile)
{
    return (tile-&maptile[0]);
}

int World::len()
{
    return side_len;
}

bool World::maximum(int x , int y)
{
    int alt = maptile[x + y * side_len].ground.altitude;    
    bool is_max = true;
    for (int i=0; i<8; i++)
    {
		int tx = x + dxo[i];
		int ty = y + dyo[i];
		is_max &= (alt >= maptile[tx + ty * side_len].ground.altitude);		
	}	
	return is_max;
}

bool World::minimum(int x , int y)
{
    int alt = maptile[x + y * side_len].ground.altitude;     
    bool is_min = true;
    for (int i=0; i<8; i++)
    {
		int tx = x + dxo[i];
		int ty = y + dyo[i];
		is_min &= (alt <= maptile[tx + ty * side_len].ground.altitude);		
	}	
	return is_min;
}

bool World::saddlepoint(int x , int y)
{
    int alt = maptile[x + y * side_len].ground.altitude;    
    int dips = 0;
    bool dip_new = alt > maptile[x + dxo[7] + (y + dyo[7])*side_len ].ground.altitude;
    bool dip_old = dip_new;
    for (int i=0; i<8; i++)
    {
		dip_new = alt > maptile[x + dxo[i]+ (y + dyo[i])*side_len].ground.altitude;
		if (dip_new && !dip_old) //We just stepped into a valley
		{
				dips++;
		}
		dip_old = dip_new;
	}	
	return dips > 1;
}

bool World::checkEdgeMin(int x , int y)
{
    int alt = maptile[x + y * side_len].ground.altitude;    
    if (x==1 || x == side_len-2)
    {
        return alt < maptile[x+1 + y * side_len].ground.altitude
            && alt < maptile[x-1 + y * side_len].ground.altitude;    
    }
    else if (y==1 || y == side_len-2)
    { 
        return alt < maptile[x + (y+1) * side_len].ground.altitude
            && alt < maptile[x + (y-1) * side_len].ground.altitude;
    }
    else
        return false;
}

/** @file lincity/world.cpp */
