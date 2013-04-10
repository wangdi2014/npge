/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "ResolveBlast.hpp"
#include "SequencesFromOther.hpp"
#include "Connector.hpp"
#include "Rest.hpp"
#include "AddBlastBlocks.hpp"
#include "CleanUp.hpp"
#include "ConSeq.hpp"
#include "DeConSeq.hpp"

namespace bloomrepeats {

ResolveBlast::ResolveBlast(BlockSetPtr source):
    Pipe(source) {
    add(new Connector, "target=other");
    add(new Rest, "target=other other=other");
    add(new ConSeq, "target=cons other=other");
    add(new Rest, "target=cons other=cons"); // seqs -> 1-blocks
    add(new AddBlastBlocks, "target=hits other=cons");
    add(new CleanUp, "target=hits other=hits");
    add(new Connector, "target=hits");
    add(new Rest, "target=hits other=hits");
    add(new SequencesFromOther, "target=target other=other");
    add(new DeConSeq, "target=target other=hits");
}

}

