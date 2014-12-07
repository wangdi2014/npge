/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_VERSION_HPP_
#define NPGE_VERSION_HPP_

#include <lua.hpp>

namespace npge {

/** Adds global table npge and field VERSION */
void init_npge_version(lua_State* L);

}

#endif
