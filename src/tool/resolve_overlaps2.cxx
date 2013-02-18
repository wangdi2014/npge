/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "process.hpp"
#include "Pipe.hpp"
#include "AddSequences.hpp"
#include "AddBlocks.hpp"
#include "OverlapsResolver2.hpp"
#include "CheckNoOverlaps.hpp"
#include "UniqueNames.hpp"
#include "Output.hpp"

using namespace bloomrepeats;

class OverlapsResolver2Pipe : public Pipe {
public:
    OverlapsResolver2() {
        set_empty_block_set();
        add(new AddSequences);
        add(new AddBlocks);
        add(new OverlapsResolver2, "target=target other=target");
        add(new CheckNoOverlaps);
        add(new UniqueNames);
        add(new Output);
    }
};

int main(int argc, char** argv) {
    return process(argc, argv, new OverlapsResolver2, "Resolve overlaps");
}

