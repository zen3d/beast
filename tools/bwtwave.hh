/* BseWaveTool - BSE Wave manipulation tool             -*-mode: c++;-*-
 * Copyright (C) 2001-2004 Tim Janik
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
#ifndef __BWT_WAVE_H__
#define __BWT_WAVE_H__

#include <bse/gsldatahandle.h>
#include <bse/gslwavechunk.h>
#include <bse/bseenums.h>
#include <string>
#include <list>

namespace BseWaveTool {
using namespace std;

class WaveChunk {
  string          temp_file;
public:
  GslDataHandle  *dhandle; /* always open */
  /*Con*/         WaveChunk();
  /*Copy*/        WaveChunk (const WaveChunk &rhs);
  WaveChunk&      operator= (const WaveChunk &);
  BseErrorType    set_dhandle_from_temporary (const string &fname,
                                              gdouble       osc_freq = -1);
  /*Des*/         ~WaveChunk();
};

struct Wave {
  guint           n_channels;
  string          name;
  list<WaveChunk> chunks;
  gchar         **wave_xinfos;
public:
  /*Con*/       Wave            (const gchar    *wave_name,
                                 guint           n_channels,
                                 gchar         **xinfos);
  void          add_chunk       (GslDataHandle  *dhandle);
  void          remove          (list<WaveChunk>::iterator it);
  void          sort            ();
  BseErrorType  store           (const string    file_name);
  /*Des*/       ~Wave           ();
};

} // BseWaveTool

#endif /* __BWT_WAVE_H__ */
