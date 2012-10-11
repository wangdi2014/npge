/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include "Sequence.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "BlockSet.hpp"
#include "AnchorFinder.hpp"
#include "CleanUp.hpp"
#include "UniqueNames.hpp"
#include "Output.hpp"
#ifndef NDEBUG
#include "Connector.hpp"
#include "OverlapsResolver.hpp"
#endif
#include "Exception.hpp"
#include "po.hpp"

using namespace bloomrepeats;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    add_general_options(desc);
    Sequence::add_input_options(desc);
    po::positional_options_description pod;
    pod.add("input-file", -1);
    AnchorFinder anchor_finder;
    anchor_finder.add_options(desc);
    Output output;
    output.add_options(desc);
    CleanUp cleanup;
    cleanup.add_options(desc);
    po::variables_map vm;
    int error = read_options(argc, argv, vm, desc, pod);
    if (error) {
        return error;
    }
    try {
        anchor_finder.apply_options(vm);
    } catch (Exception& e) {
        std::cerr << argv[0] << ": error while setting up anchor finder: "
                  << std::endl << "  " << e.what() << std::endl;
        return 255;
    }
    try {
        output.apply_options(vm);
        cleanup.apply_options(vm);
    } catch (Exception& e) {
        std::cerr << argv[0] << ": " << e.what() << std::endl;
        return 255;
    }
    BlockSetPtr block_set = boost::make_shared<BlockSet>();
    anchor_finder.set_block_set(block_set);
    std::vector<SequencePtr> seqs;
    Sequence::read_all_files(vm, seqs);
    block_set->add_sequences(seqs);
    anchor_finder.run();
    cleanup.apply(block_set);
#ifndef NDEBUG
    Connector connector;
    connector.apply(block_set);
    OverlapsResolver resolver;
    resolver.set_block_set(block_set);
    BOOST_ASSERT(!resolver.overlaps());
#endif
    UniqueNames names;
    names.apply(block_set);
    output.apply(block_set);
}

