/* BSE - Bedevilled Sound Engine
 * Copyright (C) 2004 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "bsebus.h"
#include "bsecategories.h"
#include "bsetrack.h"
#include "bsesong.h"
#include "bseengine.h"
#include "bsecsynth.h"
#include "bsesubiport.h"
#include "bsesuboport.h"
#include "bseproject.h"


/* --- parameters --- */
enum
{
  PROP_0,
  PROP_INPUTS,
  PROP_SNET,
  PROP_LEFT_VOLUME_dB,
  PROP_RIGHT_VOLUME_dB,
  PROP_MASTER_OUTPUT,
};


/* --- prototypes --- */
static gboolean bse_bus_ensure_summation (BseBus *self);

/* --- variables --- */
static gpointer		 bus_parent_class = NULL;


/* --- functions --- */
static void
bse_bus_init (BseBus *self)
{
  BSE_OBJECT_SET_FLAGS (self, BSE_SOURCE_FLAG_PRIVATE_INPUTS);
  self->left_volume = 1.0;
  self->right_volume = 1.0;
  bse_sub_synth_set_null_shortcut (BSE_SUB_SYNTH (self), TRUE);
}

static void
bse_bus_dispose (GObject *object)
{
  BseBus *self = BSE_BUS (object);
  while (self->inputs)
    bse_bus_disconnect (self, self->inputs->data);
  /* chain parent class' handler */
  G_OBJECT_CLASS (bus_parent_class)->dispose (object);
}

static void
bse_bus_finalize (GObject *object)
{
  BseBus *self = BSE_BUS (object);
  g_assert (self->inputs == NULL);
  g_assert (self->summation == NULL);
  /* chain parent class' handler */
  G_OBJECT_CLASS (bus_parent_class)->finalize (object);
}

static BseBus*
get_master (BseBus *self)
{
  BseItem *parent = BSE_ITEM (self)->parent;
  if (BSE_IS_SONG (parent))
    {
      BseSong *song = BSE_SONG (parent);
      return bse_song_find_master (song);
    }
  return NULL;
}

static void
bus_list_candidates (BseBus     *self,
                     BseItemSeq *iseq)
{
  BseItem *item = BSE_ITEM (self);
  bse_item_gather_items_typed (item, iseq, BSE_TYPE_BUS, BSE_TYPE_SONG, FALSE);
  bse_item_gather_items_typed (item, iseq, BSE_TYPE_TRACK, BSE_TYPE_SONG, FALSE);
  BseBus *master = get_master (self);
  if (master)
    bse_item_seq_remove (iseq, BSE_ITEM (master));
}

static void
bse_bus_get_candidates (BseItem               *item,
                        guint                  param_id,
                        BsePropertyCandidates *pc,
                        GParamSpec            *pspec)
{
  BseBus *self = BSE_BUS (item);
  switch (param_id)
    {
      SfiRing *ring;
    case PROP_INPUTS:
      bse_property_candidate_relabel (pc, _("Available Inputs"), _("List of available synthesis signals to be used as bus input"));
      bus_list_candidates (self, pc->items);
      ring = bse_bus_list_inputs (self);
      while (ring)
        bse_item_seq_remove (pc->items, sfi_ring_pop_head (&ring));
      /* SYNC: type partitions */
      bse_type_seq_append (pc->partitions, "BseTrack");
      bse_type_seq_append (pc->partitions, "BseBus");
      break;
    case PROP_SNET:
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, param_id, pspec);
      break;
    }
}

static void
bse_bus_set_property (GObject      *object,
                      guint         param_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  BseBus *self = BSE_BUS (object);
  switch (param_id)
    {
      SfiRing *inputs, *candidates, *ring, *saved_inputs;
      BseItemSeq *iseq;
      BseItem *parent;
      double db;
    case PROP_INPUTS:
      /* save user provided order */
      saved_inputs = bse_item_seq_to_ring (g_value_get_boxed (value));
      /* provide sorted rings: self->inputs, inputs */
      inputs = sfi_ring_sort (sfi_ring_copy (saved_inputs), sfi_compare_pointers, NULL);
      self->inputs = sfi_ring_sort (self->inputs, sfi_compare_pointers, NULL);
      /* get all input candidates */
      iseq = bse_item_seq_new();
      bus_list_candidates (self, iseq);
      candidates = sfi_ring_sort (bse_item_seq_to_ring (iseq), sfi_compare_pointers, NULL);
      bse_item_seq_free (iseq);
      /* constrain the new input list */
      ring = sfi_ring_intersection (inputs, candidates, sfi_compare_pointers, NULL);
      sfi_ring_free (candidates);
      sfi_ring_free (inputs);
      inputs = ring;
      /* eliminate stale inputs */
      ring = sfi_ring_difference (self->inputs, inputs, sfi_compare_pointers, NULL);
      while (ring)
        bse_bus_disconnect (self, sfi_ring_pop_head (&ring));
      /* add new inputs */
      ring = sfi_ring_difference (inputs, self->inputs, sfi_compare_pointers, NULL);
      while (ring)
        bse_bus_connect (self, sfi_ring_pop_head (&ring));
      sfi_ring_free (inputs);
      /* restore user provided order */
      self->inputs = sfi_ring_sort (self->inputs, sfi_compare_pointers, NULL);
      inputs = self->inputs;
      self->inputs = sfi_ring_reorder (inputs, saved_inputs, sfi_compare_pointers, NULL);
      sfi_ring_free (inputs);
      sfi_ring_free (saved_inputs);
      break;
    case PROP_SNET:
      g_object_set_property (G_OBJECT (self), "BseSubSynth::snet", value);
      break;
    case PROP_LEFT_VOLUME_dB:
      db = sfi_value_get_real (value);
      self->left_volume = bse_db_to_factor (db);
      if (self->bmodule)
        g_object_set (self->bmodule, "volume1db", db, NULL);
      break;
    case PROP_RIGHT_VOLUME_dB:
      db = sfi_value_get_real (value);
      self->right_volume = bse_db_to_factor (db);
      if (self->bmodule)
        g_object_set (self->bmodule, "volume2db", db, NULL);
      break;
    case PROP_MASTER_OUTPUT:
      parent = BSE_ITEM (self)->parent;
      if (BSE_IS_SONG (parent))
        {
          BseSong *song = BSE_SONG (parent);
          BseBus *master = bse_song_find_master (song);
          if (sfi_value_get_bool (value))
            {
              if (master != self)
                {
                  if (master)
                    bse_bus_disconnect_outputs (master);
                  bse_bus_disconnect_outputs (self);
                  bse_song_connect_master (song, self);
                  if (master)
                    g_object_notify (master, "master-output");
                }
            }
          else
            {
              if (master == self)
                bse_bus_disconnect_outputs (self);
            }
        }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
bse_bus_get_property (GObject    *object,
                      guint       param_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  BseBus *self = BSE_BUS (object);
  switch (param_id)
    {
      BseItem *parent;
      BseItemSeq *iseq;
      SfiRing *ring;
    case PROP_INPUTS:
      iseq = bse_item_seq_new();
      ring = bse_bus_list_inputs (self);
      while (ring)
        bse_item_seq_append (iseq, sfi_ring_pop_head (&ring));
      g_value_take_boxed (value, iseq);
      break;
    case PROP_SNET:
      g_object_get_property (G_OBJECT (self), "BseSubSynth::snet", value);
      break;
    case PROP_LEFT_VOLUME_dB:
      sfi_value_set_real (value, bse_db_from_factor (self->left_volume, BSE_MIN_VOLUME_dB));
      break;
    case PROP_RIGHT_VOLUME_dB:
      sfi_value_set_real (value, bse_db_from_factor (self->right_volume, BSE_MIN_VOLUME_dB));
      break;
    case PROP_MASTER_OUTPUT:
      parent = BSE_ITEM (self)->parent;
      if (BSE_IS_SONG (parent))
        {
          BseSong *song = BSE_SONG (parent);
          BseBus *master = bse_song_find_master (song);
          sfi_value_set_bool (value, self == master);
        }
      else
        sfi_value_set_bool (value, FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
bse_bus_set_parent (BseItem *item,
                    BseItem *parent)
{
  BseBus *self = BSE_BUS (item);
  /* chain parent class' handler */
  BSE_ITEM_CLASS (bus_parent_class)->set_parent (item, parent);

  while (self->inputs)
    bse_bus_disconnect (self, self->inputs->data);
  
  if (self->summation)
    {
      BseItem *sitem = BSE_ITEM (self->summation);
      self->summation = NULL;
      BseContainer *container = BSE_CONTAINER (sitem->parent);
      bse_container_remove_item (container, sitem);
    }
  if (BSE_SUB_SYNTH (self)->snet)
    {
      /* there should be snet=NULL if we have not yet a parent, and
       * snet should be set to NULL due to uncrossing before we are orphaned
       */
      g_warning ("Bus[%p] has snet[%p] in set-parent", self, BSE_SUB_SYNTH (self)->snet);
    }
}

static void
bse_bus_prepare (BseSource *source)
{
  // BseBus *iput = BSE_BUS (source);
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (bus_parent_class)->prepare (source);
}

static void
bse_bus_context_create (BseSource *source,
                        guint      context_handle,
                        BseTrans  *trans)
{
  // BseBus *iput = BSE_BUS (source);
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (bus_parent_class)->context_create (source, context_handle, trans);
}

static void
bse_bus_context_connect (BseSource *source,
                         guint      context_handle,
                         BseTrans  *trans)
{
  // BseBus *iput = BSE_BUS (source);
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (bus_parent_class)->context_connect (source, context_handle, trans);
}

static void
bse_bus_reset (BseSource *source)
{
  // BseBus *iput = BSE_BUS (source);
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (bus_parent_class)->reset (source);
}

gboolean
bse_bus_get_stack (BseBus        *self,
                   BseContainer **snetp,
                   BseSource    **vinp,
                   BseSource    **voutp)
{
  BseItem *item = BSE_ITEM (self);
  BseProject *project = bse_item_get_project (item);
  if (!BSE_SUB_SYNTH (self)->snet && project && BSE_IS_SONG (item->parent))
    {
      g_assert (self->n_effects == 0);
      bse_bus_ensure_summation (self);
      BseSNet *snet = (BseSNet*) bse_project_create_intern_csynth (project, "%BusEffectStack");
      self->vin = bse_container_new_child_bname (BSE_CONTAINER (snet), BSE_TYPE_SUB_IPORT, "%VInput", NULL);
      bse_snet_intern_child (snet, self->vin);
      BseSource *vout = bse_container_new_child_bname (BSE_CONTAINER (snet), BSE_TYPE_SUB_OPORT, "%VOutput", NULL);
      bse_snet_intern_child (snet, vout);
      self->bmodule = bse_container_new_child_bname (BSE_CONTAINER (snet), g_type_from_name ("BseBusModule"), "%Volume", NULL);
      bse_snet_intern_child (snet, self->bmodule);
      g_object_set (self->bmodule,
                    "volume1db", bse_db_from_factor (self->left_volume, BSE_MIN_VOLUME_dB),
                    "volume2db", bse_db_from_factor (self->right_volume, BSE_MIN_VOLUME_dB),
                    NULL);
      bse_source_must_set_input (vout, 0, self->bmodule, 0);
      bse_source_must_set_input (vout, 1, self->bmodule, 1);
      g_object_set (self, "BseSubSynth::snet", snet, NULL); /* no undo */
      /* connect empty effect stack */
      bse_source_must_set_input (self->bmodule, 0, self->vin, 0);
      bse_source_must_set_input (self->bmodule, 1, self->vin, 1);
    }
  if (BSE_SUB_SYNTH (self)->snet)
    {
      if (snetp)
        *snetp = (BseContainer*) BSE_SUB_SYNTH (self)->snet;
      if (vinp)
        *vinp = self->vin;
      if (voutp)
        *voutp = self->bmodule;
      return TRUE;
    }
  return FALSE;
}

static gboolean
bse_bus_ensure_summation (BseBus *self)
{
  if (!self->summation)
    {
      BseItem *item = BSE_ITEM (self);
      if (BSE_IS_SONG (item->parent))
        self->summation = bse_song_create_summation (BSE_SONG (item->parent));
      if (self->summation)
        {
          bse_source_must_set_input (BSE_SOURCE (self), 0, self->summation, 0);
          bse_source_must_set_input (BSE_SOURCE (self), 1, self->summation, 1);
        }
    }
  return self->summation != NULL;
}

static void
bus_uncross_input (BseItem *owner,
                   BseItem *item)
{
  /* delete item via procedure so deletion is recorded to undo */
  if (BSE_IS_TRACK (item))
    bse_item_exec_void (owner, "disconnect-track", item);
  else /* IS_BUS */
    bse_item_exec_void (owner, "disconnect-bus", item);
}

BseErrorType
bse_bus_connect (BseBus  *self,
                 BseItem *trackbus)
{
  BseSource *osource;
  if (BSE_IS_TRACK (trackbus))
    osource = bse_track_get_output (BSE_TRACK (trackbus));
  else if (BSE_IS_BUS (trackbus))
    osource = BSE_SOURCE (trackbus);
  else
    return BSE_ERROR_SOURCE_TYPE_INVALID;
  if (!osource || !bse_bus_ensure_summation (self) ||
      BSE_ITEM (osource)->parent != BSE_ITEM (self)->parent)    /* restrict to siblings */
    return BSE_ERROR_SOURCE_PARENT_MISMATCH;
  BseErrorType error = bse_source_set_input (self->summation, 0, osource, 0);
  if (!error)
    {
      bse_source_must_set_input (self->summation, 1, osource, 1);
      self->inputs = sfi_ring_append (self->inputs, trackbus);
      bse_item_cross_link (BSE_ITEM (self), BSE_ITEM (trackbus), bus_uncross_input);
      bse_object_proxy_notifies (trackbus, self, "inputs-changed");
      g_object_notify (self, "inputs");
      // FIXME: g_object_notify (osource, "outputs");
    }
  return error;
}

BseErrorType
bse_bus_disconnect (BseBus  *self,
                    BseItem *trackbus)
{
  BseSource *osource;
  if (BSE_IS_TRACK (trackbus))
    osource = bse_track_get_output (BSE_TRACK (trackbus));
  else if (BSE_IS_BUS (trackbus))
    osource = BSE_SOURCE (trackbus);
  else
    return BSE_ERROR_SOURCE_TYPE_INVALID;
  if (!osource || !self->summation || !sfi_ring_find (self->inputs, trackbus))
    return BSE_ERROR_SOURCE_PARENT_MISMATCH;
  bse_object_unproxy_notifies (trackbus, self, "inputs-changed");
  bse_item_cross_unlink (BSE_ITEM (self), BSE_ITEM (trackbus), bus_uncross_input);
  self->inputs = sfi_ring_remove (self->inputs, trackbus);
  BseErrorType error1 = bse_source_unset_input (self->summation, 0, osource, 0);
  BseErrorType error2 = bse_source_unset_input (self->summation, 1, osource, 1);
  g_object_notify (self, "inputs");
  // FIXME: g_object_notify (osource, "outputs");
  return error1 ? error1 : error2;
}

void
bse_bus_disconnect_outputs (BseBus *self)
{
  SfiRing *ring, *outputs = bse_bus_list_outputs (self);
  for (ring = outputs; ring; ring = sfi_ring_walk (ring, outputs))
    {
      BseErrorType error = bse_bus_disconnect (ring->data, BSE_ITEM (self));
      bse_assert_ok (error);
    }
  bse_source_clear_ochannels (BSE_SOURCE (self));       /* also disconnects master */
}

SfiRing*
bse_bus_list_inputs (BseBus *self)
{
  return sfi_ring_copy (self->inputs);
}

SfiRing*
bse_bus_list_outputs (BseBus *self)
{
  BseItem *parent = BSE_ITEM (self)->parent;
  SfiRing *outputs = NULL;
  if (BSE_IS_SONG (parent))
    {
      BseSong *song = BSE_SONG (parent);
      SfiRing *ring;
      for (ring = song->busses; ring; ring = sfi_ring_walk (ring, song->busses))
        if (ring->data != self && sfi_ring_find (BSE_BUS (ring->data)->inputs, self))
          outputs = sfi_ring_append (outputs, ring->data);
    }
  return outputs;
}

static void
bse_bus_class_init (BseBusClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  BseObjectClass *object_class = BSE_OBJECT_CLASS (class);
  BseItemClass *item_class = BSE_ITEM_CLASS (class);
  BseSourceClass *source_class = BSE_SOURCE_CLASS (class);
  guint channel_id;
  
  bus_parent_class = g_type_class_peek_parent (class);
  
  gobject_class->set_property = bse_bus_set_property;
  gobject_class->get_property = bse_bus_get_property;
  gobject_class->dispose = bse_bus_dispose;
  gobject_class->finalize = bse_bus_finalize;
  
  item_class->set_parent = bse_bus_set_parent;
  item_class->get_candidates = bse_bus_get_candidates;
  
  source_class->prepare = bse_bus_prepare;
  source_class->context_create = bse_bus_context_create;
  source_class->context_connect = bse_bus_context_connect;
  source_class->reset = bse_bus_reset;
  
  bse_object_class_add_param (object_class, _("Adjustments"),
			      PROP_LEFT_VOLUME_dB,
			      sfi_pspec_real ("left-volume-db", _("Left Volume [dB]"), _("Volume adjustment of left bus channel"),
                                              0, BSE_MIN_VOLUME_dB, BSE_MAX_VOLUME_dB,
					      0.1, SFI_PARAM_GUI ":dial"));
  bse_object_class_add_param (object_class, _("Adjustments"),
			      PROP_RIGHT_VOLUME_dB,
			      sfi_pspec_real ("right-volume-db", _("Right Volume [dB]"), _("Volume adjustment of right bus channel"),
                                              0, BSE_MIN_VOLUME_dB, BSE_MAX_VOLUME_dB,
					      0.1, SFI_PARAM_GUI ":dial"));
  bse_object_class_add_param (object_class, _("Signal Inputs"),
                              PROP_INPUTS,
                              /* SYNC: type partitions determine the order of displayed objects */
                              bse_param_spec_boxed ("inputs", _("Input Signals"),
                                                    /* TRANSLATORS: the "tracks and busses" order in this tooltip needs
                                                     * to be preserved to match the GUI order of displayed objects.
                                                     */
                                                    _("Synthesis signals (from tracks and busses) used as bus input"),
                                                    BSE_TYPE_ITEM_SEQ, SFI_PARAM_STANDARD ":item-sequence"));
  bse_object_class_add_param (object_class, NULL, PROP_SNET, bse_param_spec_object ("snet", NULL, NULL, BSE_TYPE_CSYNTH, SFI_PARAM_READWRITE ":skip-undo"));
  bse_object_class_add_param (object_class, _("Internals"),
			      PROP_MASTER_OUTPUT,
			      sfi_pspec_bool ("master-output", _("Master Output"), NULL,
                                              FALSE, SFI_PARAM_STORAGE ":skip-default"));

  channel_id = bse_source_class_add_ichannel (source_class, "left-audio-in", _("Left Audio In"), _("Left channel input"));
  g_assert (channel_id == BSE_BUS_ICHANNEL_LEFT);
  channel_id = bse_source_class_add_ichannel (source_class, "right-audio-in", _("Right Audio In"), _("Right channel input"));
  g_assert (channel_id == BSE_BUS_ICHANNEL_RIGHT);
  channel_id = bse_source_class_add_ochannel (source_class, "left-audio-out", _("Left Audio Out"), _("Left channel output"));
  g_assert (channel_id == BSE_BUS_OCHANNEL_LEFT);
  channel_id = bse_source_class_add_ochannel (source_class, "right-audio-out", _("Right Audio Out"), _("Right channel output"));
  g_assert (channel_id == BSE_BUS_OCHANNEL_RIGHT);
}

BSE_BUILTIN_TYPE (BseBus)
{
  static const GTypeInfo bus_info = {
    sizeof (BseBusClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) bse_bus_class_init,
    (GClassFinalizeFunc) NULL,
    NULL /* class_data */,
    sizeof (BseBus),
    0 /* n_preallocs */,
    (GInstanceInitFunc) bse_bus_init,
  };
  GType type = bse_type_register_static (BSE_TYPE_SUB_SYNTH,
                                         "BseBus",
                                         _("Bus implementation for songs, used to route track audio signals "
                                           "to the master output."),
                                         &bus_info);
  return type;
}
