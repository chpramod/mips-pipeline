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
 *  $Id: view.c,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 **************************************************************************
 */
#include "view.h"
#include "qops.h"
#include "misc.h"
#include "array.h"
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>


struct text_popup {
  GtkWidget *dialog;		/* dialog box */
  GtkWidget *text;		/* text */
};


enum region_types {
  T_CIRCLE, T_RECT, T_LINE
};

struct regions {
  int type;
  int fudge;
  union {
    Circle c;
    Rect r;
  } u;
  void (*click) (int type, int x, int y, void *cookie);
  void *cookie;
  struct regions *next;
};

struct view {
  GtkWidget *window;		/* top level window */
  GtkWidget *draw;		/* drawing area */
  GdkPixmap *dpix;		/* pixmap for drawing area */

  GdkGC *gc;			/* drawing color */
  GdkColor c;			/* foreground color */
  GdkColormap *cmap;		/* colormap */
  char *name;			/* color name */

  GdkFont *font;		/* font */
  GdkColor bg;			/* background color */

  void (*button_clk) (int type, int x, int y, void *cookie);
  void *cookie;
  int view_id;
  int hsize, vsize;

  void (*menubutton) (View *v);
  
  struct view *next;

  struct regions *sens;		/* sensitivity list */
  
  int flags;			/* deleted, etc */
  
  A_DECL(struct text_popup, text);
};

static struct regions *add_region (View *v)
{
  struct regions *r;

  NEW (r, struct regions);
  r->next = v->sens;
  v->sens = r;
  r->type = -1;
  r->click = NULL;
  r->cookie = NULL;

  return r;
}

void view_click_circle (View *v, Circle *c, CLICK f, void *cookie)
{
  struct regions *r;

  r = add_region (v);
  r->u.c = *c;
  r->type = T_CIRCLE;
  r->click = f;
  r->cookie = cookie;
}

void view_click_rect (View *v, Rect *c, CLICK f, void *cookie)
{
  struct regions *r;

  r = add_region (v);
  r->u.r = *c;
  r->type = T_RECT;
  r->click = f;
  r->cookie = cookie;
}

void view_click_line (View *v, Rect *c, int width, CLICK f, void *cookie)
{
  struct regions *r;

  if (width == 0) return;

  r = add_region (v);
  r->u.r = *c;
  r->type = T_LINE;
  r->fudge = width > 0 ? width : -width;
  r->click = f;
  r->cookie = cookie;
}

void view_click_clear (View *v)
{
  struct regions *r, *s;
  
  r = v->sens;
  while (r) {
    s = r;
    r = r->next;
    FREE (s);
  }
  v->sens = NULL;
}


/* sensitive region check */
static
int view_sens_check (View *v, int type, int x, int y)
{
  struct regions *r;
  double a, b, dist;
  int llx, lly, urx, ury;

  for (r = v->sens; r; r = r->next) {
    switch (r->type) {
    case T_CIRCLE:
      a = (r->u.c.x - x)*(r->u.c.x - x) + (r->u.c.y - y)*(r->u.c.y - y);
      if (a < r->u.c.radius*r->u.c.radius) {
	(*r->click)(type, x, y, r->cookie);
	return 1;
      }
      break;

    case T_RECT:
      llx = MIN(r->u.r.llx, r->u.r.urx);
      lly = MIN(r->u.r.lly, r->u.r.ury);
      urx = r->u.r.llx + r->u.r.urx - llx;
      ury = r->u.r.lly + r->u.r.ury - lly;

      if (llx <= x && lly  <= y && x < urx && y < ury) {
	(*r->click)(type, x, y, r->cookie);
	return 1;
      }
      break;

    case T_LINE:
      llx = MIN(r->u.r.llx, r->u.r.urx);
      lly = MIN(r->u.r.lly, r->u.r.ury);
      urx = r->u.r.llx + r->u.r.urx - llx;
      ury = r->u.r.lly + r->u.r.ury - lly;

      if (llx  <= x && lly  <= y && x < urx && y < ury) {
	if (r->u.r.llx == r->u.r.urx) {
	  if (x == r->u.r.llx) {
	    (*r->click)(type, x, y, r->cookie);
	    return 1;
	  }
	}
	else if (r->u.r.llx != x) {
	  a = (r->u.r.lly - r->u.r.ury + 0.0)/(r->u.r.llx -
					       r->u.r.urx+0.0);


	  dist = fabs(a*(x - r->u.r.urx) - (y - r->u.r.ury))/sqrt(1+a*a);

#if 0
	  printf ("slope: %g\n", a);
	  printf ("line: y - %d = %g * (x - %d)\n", r->u.r.ury, a, r->u.r.urx);
	  printf ("(%d,%d), dist: %g\n", x, y, dist);
#endif

	  if (dist < r->fudge) {
	    (*r->click) (type, x, y, r->cookie);
	    return 1;
	  }
	}
      }
      break;
    default:
      fatal_error ("what?");
      break;
    }
  }
  return 0;
}




#define INVALID(v) (!(v)->dpix || (v)->flags)

typedef struct view_io {
  gint tag;
  int fd;
  struct view_io *next;
} ViewIO;


static ViewIO *Ihd = NULL, *Itl;

static View *Vhd = NULL, *Vtl;

static int view_id = 0;
static int ndestroyed = 0;

static gint idle_handler_id;

static
void view_check_termination (void)
{
  if (ndestroyed == view_id && (Ihd == NULL)) {
    gtk_main_quit ();
  }
}

static
int my_thread_idle (gpointer data)
{
  extern volatile thread_t *readyQh, *readyQt;
  extern thread_t *current_process;

  if (readyQh == NULL) {
    /* only one thread in the system => delete idle handler */
    gtk_idle_remove (idle_handler_id);
  }
  thread_idle ();
  return TRUE;
}


static
void view_io_handler (gpointer data, gint source, GdkInputCondition cond)
{
  char c;
  int i;
  ch_t *ch = (ch_t *)data;
  
  i = read (source, &c, 1);
  if (i != 1) {
    view_delete_io (source);
  }
  else {
    i = c;
    ch_send (ch, &i);
  }
  thread_idle ();
}

static
void view_configure ( GtkWidget *widget, GdkEventConfigure *event,
		      gpointer data )
{
  View *v = (View *)data;
  
  if (v->dpix) {
    return;
    gdk_pixmap_unref (v->dpix);
    gdk_gc_unref (v->gc);
    //gdk_colormap_free_colors (v->cmap, &v->c, 1);
  }
  
  v->dpix = gdk_pixmap_new (widget->window,
			    widget->allocation.width,
			    widget->allocation.height,
			    -1);

  v->gc = gdk_gc_new (v->dpix);
  gdk_color_parse (v->name, &v->c);
  gdk_colormap_alloc_color (v->cmap, &v->c, FALSE, TRUE);
  gdk_gc_set_foreground (v->gc, &v->c);

  gdk_draw_rectangle (v->dpix,
		      widget->style->white_gc,
		      TRUE,
		      0, 0,
		      widget->allocation.width,
		      widget->allocation.height);
}


static
int view_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  View *v = (View *)data;

  gdk_draw_pixmap (widget->window,
		   widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		   v->dpix,
		   event->area.x, event->area.y,
		   event->area.x, event->area.y,
		   event->area.width, event->area.height);

  return FALSE;
}

void view_flush (View *v)
{
  if (INVALID (v)) return;

  gdk_draw_pixmap (v->draw->window,
		   v->draw->style->fg_gc[GTK_WIDGET_STATE (v->draw)],
		   v->dpix, 0, 0, 0, 0, v->hsize + 5, v->vsize + 35);
}


static
int view_button_press (GtkWidget *widget, GdkEventButton *event,
		       gpointer data)
{
  View *v = (View *)data;
  int type;

  if (event->x >= v->hsize)
    return FALSE;

  type = V_SINGLE;
  if (event->type == GDK_2BUTTON_PRESS) {
    type = V_DOUBLE;
  }
  else if (event->type == GDK_3BUTTON_PRESS) {
    return;
  }

  if (v->sens && view_sens_check (v, type, event->x, v->vsize - event->y - 1)){
    return FALSE;
  }

  if (!v->button_clk)
    return FALSE;

  if (event->y >= v->vsize)
    return FALSE;

  (*v->button_clk) (type, event->x, v->vsize - event->y - 1, v->cookie);

  return FALSE;
}


static
gint view_clickcallback (GtkWidget *widget, gpointer data)
{
  View *v = (View *)data;
  
  if (v->menubutton) {
    (*v->menubutton) (v);
  }
  return FALSE;
}

static
gint view_motion_notify (GtkWidget *w, GdkEventMotion *event, gpointer data)
{
  int x, y;
  GdkModifierType state;

  if (event->is_hint) {
    gdk_window_get_pointer (event->window, &x, &y, &state);
 }
  else {
    x = event->x;
    y = event->y;
    state = event->state;
  }
  return TRUE;
}



static
int view_destroy( GtkWidget *widget, gpointer data)
{
  View *v = (View *)data;
  View *prev, *t;
  ndestroyed++;
  v->flags = 1;
  view_check_termination ();
  prev = NULL;
  for (t = Vhd; t; q_step (t)) {
    if (t == v)
      break;
    prev = t;
  }
  if (!prev) {
    Vhd = Vhd->next;
  }
  else {
    prev->next = v->next;
    if (v == Vtl)
      Vtl = prev;
  }
  return TRUE;
}


static 
void view_main_thread (void)
{
  gtk_main ();
  thread_exit (0);
}


/*------------------------------------------------------------------------
 *
 *  view_init --
 *
 *    Initialize library
 *
 *------------------------------------------------------------------------
 */
void view_init (int *argc, char ***argv, void (*mainf)(void))
{
  /* nothing */
  context_unfair ();
  gtk_init (argc, argv);
  idle_handler_id = gtk_idle_add (my_thread_idle, NULL);
  thread_new (view_main_thread, DEFAULT_STACK_SIZE);
  thread_new (mainf, DEFAULT_STACK_SIZE);
  simulate (NULL);
  exit (0);
}



/*----------------------------------------------------------------------------
 *
 *  view_open_full --
 *
 *    Open a standard viewport
 *
 *  dx,dy : visible viewport
 *   x,y  : actual size
 *
 *----------------------------------------------------------------------------
 */
View *view_open_full (int dx, int dy, int x, int y)
{
  View *v;
  GtkWidget *button, *quitbox, *vbox, *sep;
  GtkWidget *scroll;

  NEW (v, View);
  v->hsize = x;
  v->vsize = y;
  v->button_clk = NULL;
  v->cookie = NULL;
  v->next = NULL;
  v->sens = NULL;
  v->flags = 0;
  v->font = NULL;
  A_INIT (v->text);

  q_ins (Vhd, Vtl, v);

  /* create a window */
  v->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (v->window), dx + 5, dy + 35);

  /*gtk_window_set_policy (GTK_WINDOW (v->window), FALSE, FALSE, TRUE);*/

  gtk_signal_connect (GTK_OBJECT (v->window), "destroy",
		      GTK_SIGNAL_FUNC (view_destroy), 
		      (gpointer)v);

  vbox = gtk_vbox_new (FALSE, 0);

  /* drawing area */
  v->dpix = NULL;
  v->draw = gtk_drawing_area_new ();
  v->name = Strdup ("black");
  v->view_id = view_id++;
  v->cmap = gdk_colormap_get_system ();

  gtk_drawing_area_size (GTK_DRAWING_AREA(v->draw), x, y);

  gtk_signal_connect (GTK_OBJECT (v->draw), "configure_event",
		       GTK_SIGNAL_FUNC (view_configure), (gpointer)v);

  gtk_signal_connect (GTK_OBJECT (v->draw), "expose_event",
		       GTK_SIGNAL_FUNC (view_expose), (gpointer)v);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scroll), 
					 v->draw);

  gtk_signal_connect (GTK_OBJECT (v->draw), "button_press_event",
		      GTK_SIGNAL_FUNC (view_button_press),
		      (gpointer)v);
  gtk_signal_connect (GTK_OBJECT (v->draw), "motion_notify_event",
		      GTK_SIGNAL_FUNC (view_motion_notify),
		      (gpointer)v);

  gtk_widget_set_events (v->draw, 
			 GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK 
			 | GDK_LEAVE_NOTIFY_MASK 
			 | GDK_POINTER_MOTION_MASK 
			 | GDK_POINTER_MOTION_HINT_MASK);
			 
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  /* separator */
  gtk_box_pack_start (GTK_BOX (vbox), sep = gtk_hseparator_new (), FALSE,
		      TRUE, 5);

  gtk_container_add (GTK_CONTAINER (v->window), vbox);

  gtk_widget_show (v->draw);
  gtk_widget_show (sep);
  gtk_widget_show (vbox);
  gtk_widget_show (scroll);

  gtk_widget_show (v->window);

  return v;
}


/*------------------------------------------------------------------------
 *
 *  view_setfg --
 *
 *    Set current foreground color
 *
 *------------------------------------------------------------------------
 */
void view_setfg (View *v, char *name)
{
  if (INVALID (v))
    return;
  
  //gdk_colormap_free_colors (v->cmap, &v->c, 1);
  FREE (v->name);
  v->name = Strdup (name);
  gdk_color_parse (v->name, &v->c);
  gdk_colormap_alloc_color (v->cmap, &v->c, FALSE, TRUE);
  gdk_gc_set_foreground (v->gc, &v->c);
}


/*------------------------------------------------------------------------
 *
 *  view_setbg --
 *
 *    Set current background color
 *
 *------------------------------------------------------------------------
 */
void view_setbg (View *v, char *name)
{
  if (INVALID (v))
    return;

  //gdk_colormap_free_colors (v->cmap, &v->bg, 1);
  gdk_color_parse (name, &v->bg);
  gdk_colormap_alloc_color (v->cmap, &v->bg, FALSE, TRUE);
  gdk_gc_set_background (v->gc, &v->bg);
}

/*------------------------------------------------------------------------
 *
 *  view_linewidth --
 *
 *    Set current line width
 *
 *------------------------------------------------------------------------
 */
void view_setlinewidth (View *v, int w)
{
  if (INVALID (v))
    return;

  gdk_gc_set_line_attributes (v->gc, w, GDK_LINE_SOLID,
			      GDK_CAP_BUTT,
			      GDK_JOIN_BEVEL);
}


/*------------------------------------------------------------------------
 *
 *  view_rect --
 *
 *    Draw rectangle using current foreground color
 *
 *------------------------------------------------------------------------
 */
void view_rect (View *v, Rect *r)
{
  if (INVALID (v))
    return;

  gdk_draw_rectangle (v->dpix,
		      v->gc,
		      TRUE,
		      r->llx, v->vsize - r->ury - 1,
		      r->urx - r->llx,
		      r->ury - r->lly);
}


void view_clear (View *v)
{
  Rect r;
  GdkGC *gc;
  
  if (INVALID(v)) return;

  gc = gdk_gc_new (v->dpix);
  gdk_color_parse (V_WHITE, &v->c);
  gdk_colormap_alloc_color (v->cmap, &v->c, FALSE, TRUE);
  gdk_gc_set_foreground (gc, &v->c);
  gdk_draw_rectangle (v->dpix, gc, TRUE, 0, 0, v->hsize + 5, v->vsize + 35);
  gdk_gc_unref (gc);
}

/*------------------------------------------------------------------------
 *
 *  view_circle --
 *
 *    Draw circle using current foreground color
 *------------------------------------------------------------------------
 */
void view_circle (View *v, Circle *c)
{
  if (INVALID (v))
    return;

  gdk_draw_arc (v->dpix, v->gc, TRUE,
		c->x - c->radius,
		(v->vsize - 1 - c->y) - c->radius,
		c->radius*2, c->radius*2,
		0,
		23040);
}

/*------------------------------------------------------------------------
 *
 *  view_circle_outline --
 *
 *    Draw circle using current foreground color
 *------------------------------------------------------------------------
 */
void view_circle_outline (View *v, Circle *c)
{
  if (INVALID (v))
    return;

  gdk_draw_arc (v->dpix, v->gc, FALSE,
		c->x - c->radius,
		(v->vsize - 1 - c->y) - c->radius,
		c->radius*2, c->radius*2,
		0,
		23040);
}

/*------------------------------------------------------------------------
 *
 *  view_box --
 *
 *    Draw rectangle using current foreground color
 *
 *------------------------------------------------------------------------
 */
void view_box (View *v, Rect *r)
{
  if (INVALID (v))
    return;

  gdk_draw_rectangle (v->dpix,
		      v->gc,
		      FALSE,
		      r->llx, v->vsize - r->ury - 1,
		      r->urx - r->llx,
		      r->ury - r->lly);
}

/*------------------------------------------------------------------------
 *
 *  view_line --
 *
 *   Draw a line from bottom left to top right of rectangle
 *
 *------------------------------------------------------------------------
 */
void view_line (View *v, Rect *r)
{
  if (INVALID (v))
    return;

  gdk_draw_line (v->dpix, v->gc,
		 r->llx, v->vsize - r->lly - 1,
		 r->urx, v->vsize - r->ury - 1);
}


void view_linearrow (View *v, Rect *r)
{
  GdkPoint points[3];
  double dist = 10;
  double linesz;
  double anchorx, anchory;
  double vx, vy;

  if (INVALID (v))
    return;

  view_line (v, r);

  points[0].x = r->urx;
  points[0].y = v->vsize - r->ury - 1;

  linesz = ((double)r->llx - r->urx)*((double)r->llx - r->urx) + 
    ((double)r->lly - r->ury)*((double)r->lly - r->ury);

  linesz = sqrt(linesz);

  if (linesz < 0.5*dist) {
    if (linesz < 5) {
      /* too small, forget it */
      return;
    }
    dist = 0.5*linesz;
  }

  anchorx = r->urx;
  anchory = v->vsize - 1 - r->ury;

  vx = (r->llx-r->urx)/linesz;
  vy = -(r->lly-r->ury)/linesz;
  
  anchorx += vx*dist;
  anchory += vy*dist;

  points[1].x = anchorx - vy*0.3*dist;
  points[1].y = anchory + vx*0.3*dist;

  points[2].x = anchorx + vy*0.3*dist;
  points[2].y = anchory - vx*0.3*dist;

#if 0
  {
    int i;
    Circle c;

    for (i=0; i < 3; i++) {
      c.x = points[i].x;
      c.y = v->vsize - 1 - points[i].y;
      c.radius = 5;
      view_circle_outline (v, &c);
    }
  }
#else
  gdk_draw_polygon (v->dpix, v->gc, TRUE, points, 3);
#endif
}


/*------------------------------------------------------------------------
 *
 *   view_text --
 *
 *      Place text at position
 *
 *------------------------------------------------------------------------
 */
void view_text (View *v, int x, int y, char *s)
{
  if (INVALID (v))
    return;

  if (!v->font) {
    v->font = gdk_font_load ("fixed");
    gdk_color_parse ("#ffffff", &v->bg);
    gdk_colormap_alloc_color (v->cmap, &v->bg, FALSE, TRUE);
  }
  
  gdk_draw_text (v->dpix, v->font, v->gc, x, v->vsize - 1 - y,
		 s, strlen(s));
}


/*------------------------------------------------------------------------
 *
 *  view_click --
 *
 *   Add callback for button click events
 *
 *------------------------------------------------------------------------
 */
void view_click (View *v, void (*f)(int,int,int,void*), void *cookie)
{
  v->button_clk = f;
  v->cookie = cookie;
}


/*------------------------------------------------------------------------
 *
 *  view_set_io --
 *
 *   Set an i/o handler
 *
 *------------------------------------------------------------------------
 */
void view_set_io (int fd, ch_t *c)
{
  ViewIO *vio;

  for (vio = Ihd; vio; q_step (vio)) {
    if (vio->fd == fd) {
      gdk_input_remove (vio->tag);
      break;
    }
  }
  if (!vio) {
    NEW (vio, ViewIO);
    vio->fd = fd;
    q_ins (Ihd, Itl, vio);
  }
  vio->tag = gdk_input_add (fd, GDK_INPUT_READ, view_io_handler, (gpointer)c);
}

/*------------------------------------------------------------------------
 *
 *  view_delete_io --
 *
 *   Set an i/o handler
 *
 *------------------------------------------------------------------------
 */
void view_delete_io (int fd)
{
  ViewIO *vio, *p;

  p = NULL;
  for (vio = Ihd; vio; q_step (vio)) {
    if (vio->fd == fd) {
      gdk_input_remove (vio->tag);
      q_delete_item (Ihd,Itl,p,vio);
      FREE (vio);
      view_check_termination ();
      return;
    }
    p = vio;
  }
}


/*
 *------------------------------------------------------------------------
 *
 *  view_popup --
 *
 *     Open a new popup window
 *
 *------------------------------------------------------------------------
 */
static void view_gtk_widget_destroy (GtkWidget *w)
{
  View *v;
  int i;
  gtk_widget_destroy (w);

  for (v = Vhd; v; v = v->next) {
    for (i=0; i < A_LEN (v->text); i++)
      if (w == v->text[i].dialog) {
	v->text[i].dialog = NULL;
	v->text[i].text = NULL;
	return;
      }
  }
}

int view_popup (View *v)
{
  GtkWidget *ok;
  int i;

  if (INVALID (v))
    return;

  if (!v->font) {
    v->font = gdk_font_load ("fixed");
    gdk_color_parse ("#ffffff", &v->bg);
    gdk_colormap_alloc_color (v->cmap, &v->bg, FALSE, TRUE);
  }

  /* insert popup id into text array */
  A_NEW (v->text, struct text_popup);
  A_NEXT(v->text).dialog = gtk_dialog_new ();
  A_NEXT(v->text).text = gtk_text_new (NULL, NULL);

  gtk_text_set_editable ((GtkText*) A_NEXT(v->text).text, FALSE);

  ok = gtk_button_new_with_label ("Close");
  gtk_signal_connect_object (GTK_OBJECT (ok), "clicked",
			     GTK_SIGNAL_FUNC (view_gtk_widget_destroy),
			     GTK_OBJECT (A_NEXT (v->text).dialog));
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG
				    (A_NEXT(v->text).dialog)->action_area),
		     ok);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG
				    (A_NEXT(v->text).dialog)->vbox),
		     A_NEXT(v->text).text);
  
  gtk_widget_show_all (A_NEXT(v->text).dialog);

  i = A_LEN (v->text);
  A_INC (v->text);
		     
  return i;
}

void view_popup_insert (View *v, int fd, char *s)
{
  gtk_text_insert (GTK_TEXT(v->text[fd].text), v->font, &v->c, &v->bg,
		   s, strlen (s));
}

void view_popup_clear (View *v, int fd)
{
  gtk_text_set_point (GTK_TEXT (v->text[fd].text), 0);
  gtk_text_forward_delete (GTK_TEXT (v->text[fd].text),
			   gtk_text_get_length (GTK_TEXT(v->text[fd].text)));
}


int view_gethsize (View *v)
{
  return v->hsize;
}


int view_getvsize (View *v)
{
  return v->vsize;
}


void view_close (View *v)
{
  gtk_widget_destroy (v->window);
}
