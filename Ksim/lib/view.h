/*************************************************************************
 *
 *  Copyright (c) 2000 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  Permission to use, copy, modify, and distribute this software
 *  and its documentation for any purpose and without fee is hereby
 *  granted, provided that the above copyright notice appear in all
 *  copies. Cornell University makes no representations
 *  about the suitability of this software for any purpose. It is
 *  provided "as is" without express or implied warranty. Export of this
 *  software outside of the United States of America may require an
 *  export license.
 *
 *  $Id: view.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 **************************************************************************
 */
#ifndef __VIEW_H__
#define __VIEW_H__

#include "thread.h"
#include "channel.h"

typedef struct {
  int llx, lly, urx, ury;
} Rect;

typedef struct {
  int x, y;
  int radius;
} Circle;

struct view;
typedef struct view View;

/* initialize view library */
void view_init (int *argc, char ***argv, void (*)(void));

/* open view window */
View *view_open_full (int dhsize, int dvsize, int hsize, int vsize);
#define view_open(h,v)  view_open_full(h,v,h,v)
void view_close (View *);

/* clear view */
void view_clear (View *v);

/* set/clear i/o channel on a file descriptor */
void view_set_io (int fd, ch_t *);
void view_delete_io (int fd);

/* callback for button click */
typedef void (*CLICK) (int type, int x, int y, void *cookie);

void view_click (View *, CLICK, void *);
void view_click_circle (View *, Circle *, CLICK, void *);
void view_click_rect (View *, Rect *, CLICK, void *);
void view_click_line (View *, Rect *, int width, CLICK, void *);
void view_click_clear (View *);

/* set drawing color */
void view_setfg (View *, char *name);
void view_setbg (View *, char *name);
void view_setlinewidth (View *v, int width);

void view_flush (View *v);

/* 
   rect = solid rectangle
   box  = outline
   line = line
*/
void view_rect (View *, Rect *);
void view_box (View *, Rect *);
void view_line (View *, Rect *);
void view_linearrow (View *, Rect *);
void view_circle (View *, Circle *);
void view_circle_outline (View *, Circle *);
void view_text (View *v, int x, int y, char *s);

/* 
   returns a popup id for this view command
*/
int view_popup (View *v);

/*
  Appends text
*/
void view_popup_insert (View *v, int fd, char *s);

/*
  Clears text
*/
void view_popup_clear (View *v, int fd);

int view_gethsize (View *v);
int view_getvsize (View *v);

/* click types */
#define V_SINGLE 0
#define V_DOUBLE 1

/* color names */
#define V_RED 		"#ff0000"
#define V_BLUE 		"#0000ff"
#define V_GREEN 	"#00ff00"
#define V_BLACK 	"#000000"
#define V_WHITE 	"#ffffff"
#define V_P_RED 	"#e55b5b"
#define V_P_BLUE 	"#8391ea"
#define V_P_GREEN 	"#6be57a"
#define V_GRAY          "#999999"

#endif /* __VIEW_H__ */
