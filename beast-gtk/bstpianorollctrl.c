/* BEAST - Bedevilled Audio System
 * Copyright (C) 2002 Tim Janik
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include "bstpianorollctrl.h"



/* --- prototypes --- */
void		controller_canvas_drag		(BstPianoRollController	*self,
						 BstPianoRollDrag	*drag);
static void	controller_update_cursor	(BstPianoRollController *self,
						 BstPianoRollTool	 tool);


/* --- variables --- */
static BswIterPartNote *clipboard_iter = NULL;


/* --- functions --- */
void
bst_piano_roll_controller_set_clipboard (BswIterPartNote *iter)
{
  if (clipboard_iter)
    bsw_iter_free (clipboard_iter);
  clipboard_iter = iter;
  if (iter)
    bsw_iter_rewind (iter);
}

BswIterPartNote*
bst_piano_roll_controller_get_clipboard (void)
{
  return clipboard_iter;
}

BstPianoRollController*
bst_piano_roll_controller_new (BstPianoRoll *proll)
{
  BstPianoRollController *self;

  g_return_val_if_fail (BST_IS_PIANO_ROLL (proll), NULL);

  self = g_new0 (BstPianoRollController, 1);
  self->proll = proll;
  self->ref_count = 1;

  self->ref_count++;
  self->note_length = 1;
  g_signal_connect_data (proll, "canvas-drag",
			 G_CALLBACK (controller_canvas_drag),
			 self, (GClosureNotify) bst_piano_roll_controller_unref,
			 G_CONNECT_SWAPPED);

  return self;
}

BstPianoRollController*
bst_piano_roll_controller_ref (BstPianoRollController *self)
{
  g_return_val_if_fail (self != NULL, NULL);
  g_return_val_if_fail (self->ref_count >= 1, NULL);

  self->ref_count++;

  return self;
}

void
bst_piano_roll_controller_unref (BstPianoRollController *self)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (self->ref_count >= 1);

  self->ref_count--;
  if (!self->ref_count)
    g_free (self);
}

void
bst_piano_roll_controller_set_obj_tools (BstPianoRollController *self,
					 BstPianoRollTool        tool1,
					 BstPianoRollTool        tool2,
					 BstPianoRollTool        tool3)
{
  g_return_if_fail (self != NULL);

  self->obj_tool1 = tool1;
  self->obj_tool2 = tool2;
  self->obj_tool3 = tool3;
}

void
bst_piano_roll_controller_set_bg_tools (BstPianoRollController *self,
					BstPianoRollTool        tool1,
					BstPianoRollTool        tool2,
					BstPianoRollTool        tool3)
{
  g_return_if_fail (self != NULL);

  self->bg_tool1 = tool1;
  self->bg_tool2 = tool2;
  self->bg_tool3 = tool3;
  controller_update_cursor (self, self->bg_tool1);
}

void
bst_piano_roll_controller_clear (BstPianoRollController *self)
{
  BswIterPartNote *iter;
  BswProxy proxy;

  g_return_if_fail (self != NULL);

  proxy = self->proll->proxy;
  iter = bsw_part_list_selected_notes (proxy);
  while (bsw_iter_n_left (iter))
    {
      BswPartNote *pnote = bsw_iter_get_part_note (iter);
      bsw_part_delete_event (proxy, pnote->id);
      bsw_iter_next (iter);
    }
  bsw_iter_free (iter);
}

void
bst_piano_roll_controller_cut (BstPianoRollController *self)
{
  BswIterPartNote *iter;
  BswProxy proxy;

  g_return_if_fail (self != NULL);

  proxy = self->proll->proxy;
  iter = bsw_part_list_selected_notes (proxy);
  while (bsw_iter_n_left (iter))
    {
      BswPartNote *pnote = bsw_iter_get_part_note (iter);
      bsw_part_delete_event (proxy, pnote->id);
      bsw_iter_next (iter);
    }
  bst_piano_roll_controller_set_clipboard (iter);
}

void
bst_piano_roll_controller_copy (BstPianoRollController *self)
{
  BswIterPartNote *iter;
  BswProxy proxy;

  g_return_if_fail (self != NULL);

  proxy = self->proll->proxy;
  iter = bsw_part_list_selected_notes (proxy);
  bst_piano_roll_controller_set_clipboard (iter);
}

void
bst_piano_roll_controller_paste (BstPianoRollController *self)
{
  BswIterPartNote *iter;
  BswProxy proxy;

  g_return_if_fail (self != NULL);

  proxy = self->proll->proxy;
  bsw_part_deselect_rectangle (proxy, 0, self->proll->max_ticks, self->proll->min_note, self->proll->max_note);
  iter = bst_piano_roll_controller_get_clipboard ();
  if (iter)
    {
      guint ptick, ctick = self->proll->max_ticks;
      gint cnote = 0;
      gint pnote;
      bst_piano_roll_get_paste_pos (self->proll, &ptick, &pnote);
      bsw_iter_rewind (iter);
      while (bsw_iter_n_left (iter))
	{
	  BswPartNote *pnote = bsw_iter_get_part_note (iter);
	  ctick = MIN (ctick, pnote->tick);
	  cnote = MAX (cnote, pnote->note);
	  bsw_iter_next (iter);
	}
      cnote = pnote - cnote;
      bsw_iter_rewind (iter);
      while (bsw_iter_n_left (iter))
	{
	  BswPartNote *pnote = bsw_iter_get_part_note (iter);
	  guint id;
	  gint note;
	  note = pnote->note + cnote;
	  if (note >= 0)
	    {
	      id = bsw_part_insert_note (proxy,
					 pnote->tick - ctick + ptick,
					 pnote->duration,
					 note,
					 pnote->fine_tune,
					 pnote->velocity);
	      if (id)
		bsw_part_select_event (proxy, id);
	    }
	  bsw_iter_next (iter);
	}
    }
}

static void
controller_update_cursor (BstPianoRollController *self,
			  BstPianoRollTool        tool)
{
  switch (tool)
    {
    case BST_PIANO_ROLL_TOOL_INSERT:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_PENCIL);
      break;
    case BST_PIANO_ROLL_TOOL_RESIZE:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_SB_H_DOUBLE_ARROW);
      break;
    case BST_PIANO_ROLL_TOOL_MOVE:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_FLEUR);
      break;
    case BST_PIANO_ROLL_TOOL_DELETE:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_TARGET);
      break;
    case BST_PIANO_ROLL_TOOL_SELECT:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_CROSSHAIR);
      break;
    case BST_PIANO_ROLL_TOOL_VSELECT:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_LEFT_SIDE);
      break;
    default:
      bst_piano_roll_set_canvas_cursor (self->proll, GDK_X_CURSOR);
      break;
    }
}

static gboolean
check_hoverlap (BswProxy part,
		guint    tick,
		guint    duration,
		gint     note,
		guint    except_tick,
		guint    except_duration)
{
  if (duration)
    {
      BswIterPartNote *iter = bsw_part_check_overlap (part, tick, duration, note);
      BswPartNote *pnote;
      
      if (bsw_iter_n_left (iter) == 0)
	{
	  bsw_iter_free (iter);
	  return FALSE;     /* no overlap */
	}
      if (bsw_iter_n_left (iter) > 1)
	{
	  bsw_iter_free (iter);
	  return TRUE;      /* definite overlap */
	}
      pnote = bsw_iter_get_part_note (iter);
      if (pnote->tick == except_tick &&
	  pnote->duration == except_duration)
	{
	  bsw_iter_free (iter);
	  return FALSE;     /* overlaps with exception */
	}
      bsw_iter_free (iter);
    }
  return TRUE;
}

static void
move_start (BstPianoRollController *self,
	    BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  if (self->obj_id)	/* got note to move */
    {
      self->xoffset = drag->start_tick - self->obj_tick;	/* drag offset */
      controller_update_cursor (self, BST_PIANO_ROLL_TOOL_MOVE);
      gxk_status_set (GXK_STATUS_WAIT, "Move Note", NULL);
      drag->state = BST_DRAG_CONTINUE;
      if (bsw_part_is_selected_event (part, self->obj_id))
	self->sel_iter = bsw_part_list_selected_notes (part);
    }
  else
    {
      gxk_status_set (GXK_STATUS_ERROR, "Move Note", "No target");
      drag->state = BST_DRAG_HANDLED;
    }
}

static void
move_group_motion (BstPianoRollController *self,
		   BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  gint new_tick, old_note, new_note, delta_tick, delta_note;

  new_tick = MAX (drag->current_tick, self->xoffset) - self->xoffset;
  new_tick = bst_piano_roll_quantize (drag->proll, new_tick);
  old_note = self->obj_note;
  new_note = drag->current_note;
  delta_tick = self->obj_tick;
  delta_note = old_note;
  delta_tick -= new_tick;
  delta_note -= new_note;
  bsw_iter_rewind (self->sel_iter);
  while (bsw_iter_n_left (self->sel_iter))
    {
      BswPartNote *pnote = bsw_iter_get_part_note (self->sel_iter);
      gint tick = pnote->tick;
      gint note = pnote->note;
      note -= delta_note;
      bsw_part_change_note (part, pnote->id,
			    MAX (tick - delta_tick, 0),
			    pnote->duration,
			    BSW_NOTE_CLAMP (note),
			    pnote->fine_tune,
			    pnote->velocity);
      bsw_iter_next (self->sel_iter);
    }
  if (drag->type == BST_DRAG_DONE)
    {
      bsw_iter_free (self->sel_iter);
      self->sel_iter = NULL;
    }
}

static void
move_motion (BstPianoRollController *self,
	     BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  guint new_tick;
  gboolean note_changed;

  if (self->sel_iter)
    {
      move_group_motion (self, drag);
      return;
    }

  new_tick = MAX (drag->current_tick, self->xoffset) - self->xoffset;
  new_tick = bst_piano_roll_quantize (drag->proll, new_tick);
  note_changed = self->obj_note != drag->current_note;
  if ((new_tick != self->obj_tick || note_changed) &&
      !check_hoverlap (part, new_tick, self->obj_duration, drag->current_note,
		       self->obj_tick, note_changed ? 0 : self->obj_duration))
    {
      if (bsw_part_delete_event (part, self->obj_id) != BSE_ERROR_NONE)
	drag->state = BST_DRAG_ERROR;
      else
	{
	  self->obj_id = bsw_part_insert_note (part, new_tick, self->obj_duration,
					       drag->current_note, self->obj_fine_tune, self->obj_velocity);
	  self->obj_tick = new_tick;
	  self->obj_note = drag->current_note;
	  if (!self->obj_id)
	    drag->state = BST_DRAG_ERROR;
	}
    }
}

static void
move_abort (BstPianoRollController *self,
	    BstPianoRollDrag       *drag)
{
  if (self->sel_iter)
    {
      bsw_iter_free (self->sel_iter);
      self->sel_iter = NULL;
    }
  gxk_status_set (GXK_STATUS_ERROR, "Move Note", "Lost Note");
}

static void
resize_start (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  if (self->obj_id)	/* got note for resize */
    {
      guint bound = self->obj_tick + self->obj_duration + 1;

      /* set the fix-point (either note start or note end) */
      if (drag->start_tick - self->obj_tick <= bound - drag->start_tick)
	self->tick_bound = bound;
      else
	self->tick_bound = self->obj_tick;
      controller_update_cursor (self, BST_PIANO_ROLL_TOOL_RESIZE);
      gxk_status_set (GXK_STATUS_WAIT, "Resize Note", NULL);
      drag->state = BST_DRAG_CONTINUE;
    }
  else
    {
      gxk_status_set (GXK_STATUS_ERROR, "Resize Note", "No target");
      drag->state = BST_DRAG_HANDLED;
    }
}

static void
resize_motion (BstPianoRollController *self,
	       BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  guint new_bound, new_tick, new_duration;

  /* calc new note around fix-point */
  new_tick = bst_piano_roll_quantize (drag->proll, drag->current_tick);
  new_bound = MAX (new_tick, self->tick_bound);
  new_tick = MIN (new_tick, self->tick_bound);
  new_duration = new_bound - new_tick;
  new_duration = MAX (new_duration, 1) - 1;

  /* apply new note size */
  if ((self->obj_tick != new_tick || new_duration != self->obj_duration) &&
      !check_hoverlap (part, new_tick, new_duration, self->obj_note,
		       self->obj_tick, self->obj_duration))
    {
      if (self->obj_id)
	{
	  BseErrorType error = bsw_part_delete_event (part, self->obj_id);
	  if (error)
	    drag->state = BST_DRAG_ERROR;
	  self->obj_id = 0;
	}
      if (new_duration && drag->state != BST_DRAG_ERROR)
	{
	  self->obj_id = bsw_part_insert_note (part, new_tick, new_duration,
					       self->obj_note, self->obj_fine_tune, self->obj_velocity);
	  self->obj_tick = new_tick;
	  self->obj_duration = new_duration;
	  if (!self->obj_id)
	    drag->state = BST_DRAG_ERROR;
	}
    }
}

static void
resize_abort (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Resize Note", "Lost Note");
}

static void
delete_start (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  if (self->obj_id)	/* got note to delete */
    {
      BseErrorType error = bsw_part_delete_event (part, self->obj_id);
      bst_status_eprintf (error, "Delete Note");
    }
  else
    gxk_status_set (GXK_STATUS_ERROR, "Delete Note", "No target");
  drag->state = BST_DRAG_HANDLED;
}

static void
insert_start (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  BswErrorType error = BSW_ERROR_NO_TARGET;
  if (drag->start_valid)
    {
      guint qtick = bst_piano_roll_quantize (drag->proll, drag->start_tick);
      guint duration = drag->proll->ppqn * 4 / self->note_length;
      if (check_hoverlap (part, qtick, duration, drag->start_note, 0, 0))
	error = BSW_ERROR_INVALID_OVERLAP;
      else
	{
	  bsw_part_insert_note (part, qtick, duration, drag->start_note, 0, 1.0);
	  error = BSW_ERROR_NONE;
	}
    }
  bst_status_eprintf (error, "Insert Note");
  drag->state = BST_DRAG_HANDLED;
}

static void
select_start (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  drag->start_tick = bst_piano_roll_quantize (drag->proll, drag->start_tick);
  bst_piano_roll_set_view_selection (drag->proll, drag->start_tick, 0, 0, 0);
  gxk_status_set (GXK_STATUS_WAIT, "Select Region", NULL);
  drag->state = BST_DRAG_CONTINUE;
}

static void
select_motion (BstPianoRollController *self,
	       BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  guint start_tick = MIN (drag->start_tick, drag->current_tick);
  guint end_tick = MAX (drag->start_tick, drag->current_tick);
  gint min_note = MIN (drag->start_note, drag->current_note);
  gint max_note = MAX (drag->start_note, drag->current_note);

  bst_piano_roll_set_view_selection (drag->proll, start_tick, end_tick - start_tick, min_note, max_note);
  if (drag->type == BST_DRAG_DONE)
    {
      bsw_part_select_rectangle_exclusive (part, start_tick, end_tick - start_tick, min_note, max_note);
      bst_piano_roll_set_view_selection (drag->proll, 0, 0, 0, 0);
    }
}

static void
select_abort (BstPianoRollController *self,
	      BstPianoRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Select Region", "Aborted");
  bst_piano_roll_set_view_selection (drag->proll, 0, 0, 0, 0);
}

static void
vselect_start (BstPianoRollController *self,
	       BstPianoRollDrag       *drag)
{
  drag->start_tick = bst_piano_roll_quantize (drag->proll, drag->start_tick);
  bst_piano_roll_set_view_selection (drag->proll, drag->start_tick, 0, drag->proll->min_note, drag->proll->max_note);
  gxk_status_set (GXK_STATUS_WAIT, "Vertical Select", NULL);
  drag->state = BST_DRAG_CONTINUE;
}

static void
vselect_motion (BstPianoRollController *self,
		BstPianoRollDrag       *drag)
{
  BswProxy part = self->proll->proxy;
  guint start_tick = MIN (drag->start_tick, drag->current_tick);
  guint end_tick = MAX (drag->start_tick, drag->current_tick);

  bst_piano_roll_set_view_selection (drag->proll, start_tick, end_tick - start_tick,
				     drag->proll->min_note, drag->proll->max_note);
  if (drag->type == BST_DRAG_DONE)
    {
      bsw_part_select_rectangle_exclusive (part, start_tick, end_tick - start_tick,
					   drag->proll->min_note, drag->proll->max_note);
      bst_piano_roll_set_view_selection (drag->proll, 0, 0, 0, 0);
    }
}

static void
vselect_abort (BstPianoRollController *self,
	       BstPianoRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Vertical Region", "Aborted");
  bst_piano_roll_set_view_selection (drag->proll, 0, 0, 0, 0);
}

#if 0
static void
generic_abort (BstPianoRollController *self,
	       BstPianoRollDrag       *drag)
{
  gxk_status_set (GXK_STATUS_ERROR, "Abortion", NULL);
}
#endif

typedef void (*DragFunc) (BstPianoRollController *,
			  BstPianoRollDrag       *);

void
controller_canvas_drag (BstPianoRollController *self,
			BstPianoRollDrag       *drag)
{
  static struct {
    BstPianoRollTool tool;
    DragFunc start, motion, abort;
  } tool_table[] = {
    { BST_PIANO_ROLL_TOOL_INSERT,	insert_start,	NULL,		NULL,		},
    { BST_PIANO_ROLL_TOOL_RESIZE,	resize_start,	resize_motion,	resize_abort,	},
    { BST_PIANO_ROLL_TOOL_MOVE,		move_start,	move_motion,	move_abort,	},
    { BST_PIANO_ROLL_TOOL_DELETE,	delete_start,	NULL,		NULL,		},
    { BST_PIANO_ROLL_TOOL_SELECT,	select_start,	select_motion,	select_abort,	},
    { BST_PIANO_ROLL_TOOL_VSELECT,	vselect_start,	vselect_motion,	vselect_abort,	},
  };
  guint i;

  if (drag->type == BST_DRAG_START)
    {
      BstPianoRollTool tool = BST_PIANO_ROLL_TOOL_NONE;
      BswIterPartNote *iter;

      /* setup drag data */
      iter = bsw_part_get_notes (drag->proll->proxy, drag->start_tick, drag->start_note);
      if (bsw_iter_n_left (iter))
	{
	  BswPartNote *pnote = bsw_iter_get_part_note (iter);
	  self->obj_id = pnote->id;
	  self->obj_tick = pnote->tick;
	  self->obj_duration = pnote->duration;
	  self->obj_note = pnote->note;
	  self->obj_fine_tune = pnote->fine_tune;
	  self->obj_velocity = pnote->velocity;
	}
      else
	{
	  self->obj_id = 0;
	  self->obj_tick = 0;
	  self->obj_duration = 0;
	  self->obj_note = 0;
	  self->obj_fine_tune = 0;
	  self->obj_velocity = 0;
	}
      if (self->sel_iter)
	g_warning ("leaking old drag selection (%p)", self->sel_iter);
      self->sel_iter = NULL;
      self->xoffset = 0;
      self->tick_bound = 0;
      bsw_iter_free (iter);

      /* find drag tool */
      tool = BST_PIANO_ROLL_TOOL_NONE;
      if (self->obj_id)		/* have object */
	switch (drag->button)
	  {
	  case 1:	tool = self->obj_tool1;	break;
	  case 2:	tool = self->obj_tool2;	break;
	  case 3:	tool = self->obj_tool3;	break;
	  }
      else
	switch (drag->button)
	  {
	  case 1:	tool = self->bg_tool1;	break;
	  case 2:	tool = self->bg_tool2;	break;
	  case 3:	tool = self->bg_tool3;	break;
	  }
      for (i = 0; i < G_N_ELEMENTS (tool_table); i++)
	if (tool_table[i].tool == tool)
	  break;
      self->tool_index = i;
      if (i >= G_N_ELEMENTS (tool_table))
	return;		/* unhandled */
    }
  i = self->tool_index;
  g_return_if_fail (i < G_N_ELEMENTS (tool_table));
  switch (drag->type)
    {
    case BST_DRAG_START:
      if (tool_table[i].start)
	tool_table[i].start (self, drag);
      break;
    case BST_DRAG_MOTION:
    case BST_DRAG_DONE:
      if (tool_table[i].motion)
	tool_table[i].motion (self, drag);
      break;
    case BST_DRAG_ABORT:
      if (tool_table[i].abort)
	tool_table[i].abort (self, drag);
      break;
    }
  if (drag->type == BST_DRAG_DONE ||
      drag->type == BST_DRAG_ABORT)
    controller_update_cursor (self, self->bg_tool1);
}
