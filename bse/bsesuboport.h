/* BSE - Better Sound Engine
 * Copyright (C) 1999, 2000-2002 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License should ship along
 * with this library; if not, see http://www.gnu.org/copyleft/.
 */
#ifndef __BSE_SUB_OPORT_H__
#define __BSE_SUB_OPORT_H__

#include <bse/bsesource.h>
#include <bse/bseengine.h>

G_BEGIN_DECLS

/* --- object type macros --- */
#define BSE_TYPE_SUB_OPORT		(BSE_TYPE_ID (BseSubOPort))
#define BSE_SUB_OPORT(object)		(G_TYPE_CHECK_INSTANCE_CAST ((object), BSE_TYPE_SUB_OPORT, BseSubOPort))
#define BSE_SUB_OPORT_CLASS(class)	(G_TYPE_CHECK_CLASS_CAST ((class), BSE_TYPE_SUB_OPORT, BseSubOPortClass))
#define BSE_IS_OPORT(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), BSE_TYPE_SUB_OPORT))
#define BSE_IS_OPORT_CLASS(class)	(G_TYPE_CHECK_CLASS_TYPE ((class), BSE_TYPE_SUB_OPORT))
#define BSE_SUB_OPORT_GET_CLASS(object)	(G_TYPE_INSTANCE_GET_CLASS ((object), BSE_TYPE_SUB_OPORT, BseSubOPortClass))

#define	BSE_SUB_OPORT_N_PORTS	(4)

/* --- BseSubOPort module --- */
typedef struct _BseSubOPort      BseSubOPort;
typedef struct _BseSubOPortClass BseSubOPortClass;
struct _BseSubOPort
{
  BseSource	 parent_object;
  gchar	       **output_ports;
};
struct _BseSubOPortClass
{
  BseSourceClass parent_class;
};

G_END_DECLS

#endif /* __BSE_SUB_OPORT_H__ */
