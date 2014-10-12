/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_PRINT_MUTATIONS_HPP_
#define NPGE_PRINT_MUTATIONS_HPP_

#include <boost/function.hpp>

#include "AbstractOutput.hpp"
#include "global.hpp"

namespace npge {

/** Mutation description */
struct Mutation {
    Fragment* fragment;
    int pos;
    char change;
};

/** Function processing a mutation */
typedef boost::function<void(const Mutation&)> MutationHandler;

/** Find all mutations in block */
class PrintMutations : public AbstractOutput {
public:
    /** Constructor */
    PrintMutations();

    /** Find mutations and calls f() for each mutation */
    void find_mutations(const Block* block, const MutationHandler& f) const;

    /** Print table.
    Table columns:
        - block
        - fr
        - pos
        - change
    */
    void print_block(std::ostream& o, Block* block) const;

    void print_header(std::ostream& o) const;

protected:
    const char* name_impl() const;
};

}

#endif

