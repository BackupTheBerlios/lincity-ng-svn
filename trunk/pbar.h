/* pbar.h: handles rate-of-change indicators 
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * Portions copyright (c) 2001 Corey Keasling.
 * ---------------------------------------------------------------------- */

#ifndef _pbar_h
#define _pbar_h

void init_pbars (void);
void pbars_full_refresh (void);
void init_pbar_text (void);
void draw_pbar (Rect* b, char* graphic);
void draw_pbars (void);
void clear_pbar_text (Rect* pbar);
void write_pbar_int (Rect* b, int val);
void write_pbar_text (Rect* b, char * s);
int pbar_adjust_pop (int diff);
int pbar_adjust_tech (int diff);
int pbar_adjust_food (int diff);
int pbar_adjust_jobs (int diff);
int pbar_adjust_coal (int diff);
int pbar_adjust_goods (int diff);
int pbar_adjust_ore (int diff);
int pbar_adjust_steel (int diff);
int pbar_adjust_money (int diff);
void refresh_pbars (void);
void update_pbar_pop (int pop);
void update_pbar_tech (int tech);
void update_pbar_food (int food);
void update_pbar_jobs (int jobs);
void update_pbar_coal (int coal);
void update_pbar_goods (int goods);
void update_pbar_ore (int ore);
void update_pbar_steel (int steel);
void update_pbar_money (int money);
int compute_pbar_offset (Rect* b, int val);
void draw_pbar_new (Rect* b, int total, int val, int oldval);
void pbar_mouse(int rawx, int rawy, int button);

/* XXX: WCK: These are actually defined in shrglobs.c, maybe in pbar.c? */
extern int pbar_pop[12];
extern int pbar_tech[12];
extern int pbar_food[12];
extern int pbar_jobs[12];
extern int pbar_coal[12];
extern int pbar_goods[12];
extern int pbar_ore[12];
extern int pbar_steel[12];
extern int pbar_money[12];

/* Constants */

#define PBAR_POP_X      (PBAR_AREA_X + 4)
#define PBAR_POP_Y      (PBAR_AREA_Y + 4)
#define PBAR_TECH_X     PBAR_POP_X
#define PBAR_TECH_Y     PBAR_POP_Y+(PBAR_H+1)
#define PBAR_FOOD_X     PBAR_POP_X
#define PBAR_FOOD_Y     PBAR_POP_Y+(PBAR_H+1)*2
#define PBAR_JOBS_X     PBAR_POP_X
#define PBAR_JOBS_Y     PBAR_POP_Y+(PBAR_H+1)*3
#define PBAR_COAL_X     PBAR_POP_X
#define PBAR_COAL_Y     PBAR_POP_Y+(PBAR_H+1)*4
#define PBAR_GOODS_X    PBAR_POP_X
#define PBAR_GOODS_Y    PBAR_POP_Y+(PBAR_H+1)*5
#define PBAR_ORE_X      PBAR_POP_X
#define PBAR_ORE_Y      PBAR_POP_Y+(PBAR_H+1)*6
#define PBAR_STEEL_X    PBAR_POP_X
#define PBAR_STEEL_Y    PBAR_POP_Y+(PBAR_H+1)*7
#define PBAR_MONEY_X    PBAR_POP_X
#define PBAR_MONEY_Y    PBAR_POP_Y+(PBAR_H+1)*8

#endif
