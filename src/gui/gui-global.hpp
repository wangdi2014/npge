/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_GUI_GLOBAL_HPP_
#define NPGE_GUI_GLOBAL_HPP_

#include <map>

#include "global.hpp"
#include "SortedVector.hpp"
#include "FragmentCollection.hpp"

using namespace npge;

class AlignmentView;
class AlignmentModel;
class BSAModel;
class BlockSetWidget;

typedef std::vector<const Block*> ConstBlocks;
typedef SortedVector<const Block*> SortedBlocks;

typedef FragmentCollection<Fragment*, Fragments> S2F;

typedef std::map<Sequence*, Fragment*> Seq2Fragment;

#endif

