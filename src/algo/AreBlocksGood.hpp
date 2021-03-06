/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_ARE_BLOCKS_GOOD_HPP_
#define NPGE_ARE_BLOCKS_GOOD_HPP_

#include <iosfwd>

#include "Processor.hpp"
#include "FileWriter.hpp"
#include "global.hpp"

namespace npge {

class Filter;
class MoveGaps;
class CutGaps;

/** Print if blockset is a good pangenome.
Requirements of a good pangenome:
 - no overlapping blocks.
 - CutGaps and MoveGaps applied to each block.
 - length of any fragment (not from 1-fragment blocks) >= limit.
 - alignment is defined for each block of >= 2 fragments.
 - identity of any block >= limit.
 - sequences are covered entirely by blocks (including 1-fragment blocks).
*/
class AreBlocksGood : public Processor {
public:
    /** Constructor */
    AreBlocksGood();

    /** Get output stream */
    std::ostream& get_out() const;

    /** Return if all blocks are good and print messages to output */
    bool are_blocks_good() const;

protected:
    void run_impl() const;

    const char* name_impl() const;

private:
    FileWriter file_writer_;
    CutGaps* cut_gaps_;
    MoveGaps* move_gaps_;
    Filter* filter_;
};

}

#endif

