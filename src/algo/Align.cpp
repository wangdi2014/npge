/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/foreach.hpp>

#include "Align.hpp"
#include "ExternalAligner.hpp"
#include "MoveGaps.hpp"
#include "CutGaps.hpp"
#include "Filter.hpp"

namespace bloomrepeats {

class AlignLoop : public Pipe {
public:
    AlignLoop() {
        set_max_loops(-1);
        add(new MoveGaps);
        add(new CutGaps);
        add(new Filter);
    }
};

Align::Align() {
    add(new Filter);
    add(new ExternalAligner);
    add(new AlignLoop);
}

}

