/* DavBassFilter - DAV Bass Filter
 * Copyright (c) 1999, 2000 David A. Bartold
 *
 * This is a TB-303 filter clone based on the VCF303 portions of
 * gsyn v0.2.  Code in update_locals() is copyright (c) 1998 Andy Sloane.
 *
 * Filter algorithm in function recalc_a_b() is based on the one
 * described in Musical Applications of Microprocessors by Hal Chamberlin.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "davbassfilter.h"

#include <bse/bsechunk.h>
#include <bse/bsehunkmixer.h>

/* --- parameters --- */
enum
{
  PARAM_0,
  PARAM_TRIGGER,
  PARAM_CUTOFF_FREQ,
  PARAM_RESONANCE,
  PARAM_ENV_MOD,
  PARAM_DECAY
};


/* --- prototypes --- */
static void        dav_bass_filter_init             (DavBassFilter       *filter);
static void        dav_bass_filter_class_init       (DavBassFilterClass  *class);
static void        dav_bass_filter_class_destroy    (DavBassFilterClass  *class);
static void        dav_bass_filter_do_shutdown      (BseObject           *object);
static void        dav_bass_filter_set_param        (DavBassFilter       *filter,
						     BseParam             *param,
						     guint                 param_id);
static void        dav_bass_filter_get_param        (DavBassFilter       *filter,
						     BseParam             *param,
						     guint                 param_id);
static void        dav_bass_filter_prepare          (BseSource           *source,
						     BseIndex              index);
static BseChunk*   dav_bass_filter_calc_chunk       (BseSource           *source,
						     guint                 ochannel_id);
static void        dav_bass_filter_reset            (BseSource           *source);
static inline void dav_bass_filter_update_locals    (DavBassFilter       *filter);


/* --- variables --- */
static BseType           type_id_bass_filter = 0;
static gpointer          parent_class = NULL;
static const BseTypeInfo type_info_bass_filter = {
  sizeof (DavBassFilterClass),
  
  (BseBaseInitFunc) NULL,
  (BseBaseDestroyFunc) NULL,
  (BseClassInitFunc) dav_bass_filter_class_init,
  (BseClassDestroyFunc) dav_bass_filter_class_destroy,
  NULL /* class_data */,
  
  sizeof (DavBassFilter),
  0 /* n_preallocs */,
  (BseObjectInitFunc) dav_bass_filter_init,
};


/* --- functions --- */

static inline void
recalc_a_b (DavBassFilter *filter)
{
  gfloat whopping, k;
  
  whopping = filter->e0 + filter->c0;
  k = exp (-whopping / filter->resonance);
  
  filter->a = 2.0 * cos (2.0 * whopping) * k;
  filter->b = -k * k;
}

static inline void
dav_bass_filter_update_locals (DavBassFilter *filter)
{
  gfloat d;
  
  /* Update vars given envmod, cutoff, and reso. */
  filter->e0 = exp (5.613 - 0.8 * filter->envmod + 2.1553 * filter->cutoff - 0.7696 * (1.0 - filter->reso));
  filter->e1 = exp (6.109 + 1.5876 * filter->envmod + 2.1553 * filter->cutoff - 1.2 * (1.0 - filter->reso));
  
  filter->e0 *= PI / BSE_MIX_FREQ;
  filter->e1 *= PI / BSE_MIX_FREQ;
  
  filter->e1 -= filter->e0;
  
  /* Update decay given envdecay. */
  d = 0.2 + (2.3 * filter->envdecay);
  d *= BSE_MIX_FREQ;
  d = pow (0.1, 1.0 / d);
  filter->decay = pow (d, 64);
  
  /* Update resonance. */
  filter->resonance = exp (-1.20 + 3.455 * filter->reso);
  
  recalc_a_b (filter);
}

static void
dav_bass_filter_class_init (DavBassFilterClass *class)
{
  BseObjectClass *object_class;
  BseSourceClass *source_class;
  guint ochannel_id, ichannel_id;
  
  parent_class = bse_type_class_peek (BSE_TYPE_SOURCE);
  object_class = BSE_OBJECT_CLASS (class);
  source_class = BSE_SOURCE_CLASS (class);
  
  object_class->set_param = (BseObjectSetParamFunc) dav_bass_filter_set_param;
  object_class->get_param = (BseObjectGetParamFunc) dav_bass_filter_get_param;
  object_class->shutdown = dav_bass_filter_do_shutdown;
  
  source_class->prepare = dav_bass_filter_prepare;
  source_class->calc_chunk = dav_bass_filter_calc_chunk;
  source_class->reset = dav_bass_filter_reset;
  
  bse_object_class_add_param (object_class, "Trigger", PARAM_TRIGGER,
                              bse_param_spec_bool ("trigger", "Trigger filter",
                                                   "Trigger the filter",
						   FALSE, BSE_PARAM_GUI));
  
  bse_object_class_add_param (object_class, "Parameters", PARAM_CUTOFF_FREQ,
                              bse_param_spec_float ("cutoff_freq", "Cutoff [%]",
						    "Set the cutoff frequency",
                                                    0.0, 100.0, 0.1, 50.0,
                                                    BSE_PARAM_DEFAULT | BSE_PARAM_HINT_SCALE));
  
  bse_object_class_add_param (object_class, "Parameters", PARAM_RESONANCE,
                              bse_param_spec_float ("resonance", "Resonance [%]",
                                                    "Set the amount of resonance",
                                                    0.0, 100.0, 0.1, 99.5,
                                                    BSE_PARAM_DEFAULT | BSE_PARAM_HINT_SCALE));
  
  bse_object_class_add_param (object_class, "Parameters", PARAM_ENV_MOD,
                              bse_param_spec_float ("env_mod", "Envelope Modulation [%]",
                                                    "Set the envelope magnitude",
                                                    0.0, 100.0, 0.1, 90.0,
                                                    BSE_PARAM_DEFAULT | BSE_PARAM_HINT_SCALE));
  
  bse_object_class_add_param (object_class, "Parameters", PARAM_DECAY,
                              bse_param_spec_float ("decay", "Decay [%]",
                                                    "Set the decay length",
                                                    0.0, 100.0, 0.1, 20.0,
                                                    BSE_PARAM_DEFAULT | BSE_PARAM_HINT_SCALE));
  
  ochannel_id = bse_source_class_add_ochannel (source_class, "mono_out", "BassFilter Output", 1);
  g_assert (ochannel_id == DAV_BASS_FILTER_OCHANNEL_MONO);
  
  ichannel_id = bse_source_class_add_ichannel (source_class, "mono_in", "Sound Input", 1, 1);
  g_assert (ichannel_id == DAV_BASS_FILTER_ICHANNEL_MONO);
}

static void
dav_bass_filter_class_destroy (DavBassFilterClass *class)
{
}

static void
dav_bass_filter_init (DavBassFilter *filter)
{
  filter->d1 = 0.0;
  filter->d2 = 0.0;
  filter->reso = 0.995;
  filter->envmod = 0.9;
  filter->envdecay = 0.2;
  filter->envpos = 0;
  filter->cutoff = 0.5;
  filter->a = 0.0;
  filter->b = 0.0;
  filter->c0 = 0.0;
  
  dav_bass_filter_update_locals (filter);
}

static void
dav_bass_filter_do_shutdown (BseObject *object)
{
  DavBassFilter *filter;
  
  filter = DAV_BASS_FILTER (object);
  
  /* chain parent class' shutdown handler */
  BSE_OBJECT_CLASS (parent_class)->shutdown (object);
}

static void
dav_bass_filter_set_param (DavBassFilter *filter,
			   BseParam      *param,
			   guint          param_id)
{
  switch (param_id)
    {
    case PARAM_TRIGGER:
      dav_bass_filter_update_locals (filter);
      
      /* Reset filter delta freq. */
      filter->c0 = filter->e1;
      filter->envpos = 0;
      break;
      
    case PARAM_CUTOFF_FREQ:
      filter->cutoff = param->value.v_float / 100.0;
      dav_bass_filter_update_locals (filter);
      break;
      
    case PARAM_RESONANCE:
      filter->reso = param->value.v_float / 100.0;
      dav_bass_filter_update_locals (filter);
      break;
      
    case PARAM_ENV_MOD:
      filter->envmod = param->value.v_float / 100.0;
      dav_bass_filter_update_locals (filter);
      break;
      
    case PARAM_DECAY:
      filter->envdecay = param->value.v_float / 100.0;
      dav_bass_filter_update_locals (filter);
      break;
      
    default:
      BSE_UNHANDLED_PARAM_ID (filter, param, param_id);
      break;
    }
}

static void
dav_bass_filter_get_param (DavBassFilter *filter,
			   BseParam      *param,
			   guint          param_id)
{
  switch (param_id)
    {
    case PARAM_TRIGGER:
      param->value.v_bool = FALSE;
      break;
      
    case PARAM_CUTOFF_FREQ:
      param->value.v_float = filter->cutoff * 100.0;
      break;
      
    case PARAM_RESONANCE:
      param->value.v_float = filter->reso * 100.0;
      break;
      
    case PARAM_ENV_MOD:
      param->value.v_float = filter->envmod * 100.0;
      break;
      
    case PARAM_DECAY:
      param->value.v_float = filter->envdecay * 100.0;
      break;
      
    default:
      BSE_UNHANDLED_PARAM_ID (filter, param, param_id);
      break;
    }
}

static void
dav_bass_filter_prepare (BseSource *source,
			 BseIndex   index)
{
  DavBassFilter *filter = DAV_BASS_FILTER (source);
  
  dav_bass_filter_update_locals (filter);
  
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (parent_class)->prepare (source, index);
}

static BseChunk*
dav_bass_filter_calc_chunk (BseSource *source,
			    guint      ochannel_id)
{
  DavBassFilter *filter = DAV_BASS_FILTER (source);
  BseSampleValue *hunk;
  BseSourceInput *input;
  BseChunk *input_chunk;
  BseSampleValue *inputs;
  gint i;
  gfloat c;
  gfloat value;
  
  g_return_val_if_fail (ochannel_id == DAV_BASS_FILTER_OCHANNEL_MONO, NULL);
  
  hunk = bse_hunk_alloc (1);
  
  input = bse_source_get_input (source, DAV_BASS_FILTER_ICHANNEL_MONO);
  
  if (input != NULL)
    {
      input_chunk = bse_source_ref_chunk (input->osource, input->ochannel_id, source->index);
      inputs = input_chunk->hunk;
    }
  else
    {
      input_chunk = NULL;
      inputs = NULL;
    }
  
  for (i = 0; i < BSE_TRACK_LENGTH; i++)
    {
      if (input_chunk == NULL)
        {
          value = filter->a * filter->d1 + filter->b * filter->d2;
        }
      else
        {
          c = (1.0 - filter->a - filter->b) * 0.2;
          value = filter->a * filter->d1 + filter->b * filter->d2 + c * inputs[i];
        }
      
      hunk [i] = BSE_CLIP_SAMPLE_VALUE (value);
      
      filter->d2 = filter->d1;
      filter->d1 = value;
      
      filter->envpos++;
      
      if (filter->envpos >= 64)
        {
          filter->envpos = 0;
          filter->c0 *= filter->decay;
          recalc_a_b (filter);
        }
    }
  
  if (input_chunk != NULL)
    bse_chunk_unref (input_chunk);
  
  return bse_chunk_new_orphan (1, hunk);
}

static void
dav_bass_filter_reset (BseSource *source)
{
  /* chain parent class' handler */
  BSE_SOURCE_CLASS (parent_class)->reset (source);
}

/* --- Export to DAV --- */
#include "./icons/noicon.c"
BSE_EXPORTS_BEGIN (BSE_PLUGIN_NAME);
BSE_EXPORT_OBJECTS = {
  { &type_id_bass_filter, "DavBassFilter", "BseSource",
    "DavBassFilter is a low-pass resonant filter",
    &type_info_bass_filter,
    "/Source/BassFilter",
    { NOICON_BYTES_PER_PIXEL | BSE_PIXDATA_1BYTE_RLE,
      NOICON_WIDTH, NOICON_HEIGHT,
      NOICON_RLE_PIXEL_DATA, },
  },
  { NULL, },
};
BSE_EXPORTS_END;
