/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_CONSEQ_HPP_
#define NPGE_CONSEQ_HPP_

#include "BlocksJobs.hpp"

namespace npge {

/** Add consensus sequences, produced from blocks of source blockset.
Depends on UniqueNames. Blocks must be aligned.
*/
class ConSeq : public BlocksJobs {
public:
    /** Constructor */
    ConSeq(const BlockSetPtr& source = BlockSetPtr());

protected:
    ThreadData* before_thread_impl() const;

    void process_block_impl(Block* b, ThreadData* d) const;

    void after_thread_impl(ThreadData* d) const;

    const char* name_impl() const;
};

}

#endif

