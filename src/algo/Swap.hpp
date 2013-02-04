/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_SWAP_HPP_
#define BR_SWAP_HPP_

#include "Processor.hpp"
#include "OtherBlockSet.hpp"

namespace bloomrepeats {

/** Exchange two block sets.
Wrapper of BlockSet::swap().
*/
class Swap : public Processor, public OtherBlockSet {
public:
    /** Constructor
    \param other BlockSet, blocks of which will be exchanged with block_set().
    */
    Swap(const BlockSetPtr& other);

protected:
    /** Apply the action */
    bool run_impl() const;
};

}

#endif

