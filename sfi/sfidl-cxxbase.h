/* SFI - Synthesis Fusion Kit Interface
 * Copyright (C) 2003 Stefan Westerfeld
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
#ifndef __SFIDL_CXXBASE_H__
#define __SFIDL_CXXBASE_H__

#include "sfidl-cbase.h"
#include "sfidl-namespace.h"

namespace Sfidl {

  class CodeGeneratorCxxBase : public CodeGeneratorCBase {
  protected:
    std::string untyped_pspec_constructor (const Param &param);
    std::string typed_pspec_constructor   (const Param &param);

  public:
    CodeGeneratorCxxBase (const Parser& parser) : CodeGeneratorCBase (parser) {
    }
  };

};

#endif  /* __SFIDL_CXXBASE_H__ */

/* vim:set ts=8 sts=2 sw=2: */
