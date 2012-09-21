/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_BLOCK_SET_HPP_
#define BR_BLOCK_SET_HPP_

#include <iosfwd>
#include <set>
#include <boost/pool/pool_alloc.hpp>

#include "global.hpp"

namespace bloomrepeats {

/** Container of blocks.
*/
class BlockSet {
public:
    /** Type of implementation container.
    Do not rely on ths type!
    */
    typedef std::set < BlockPtr, std::less<BlockPtr>,
            boost::fast_pool_allocator<void> > Impl;

    /** Iterator */
    typedef Impl::iterator iterator;

    /** Constant iterator */
    typedef Impl::const_iterator const_iterator;

    /** Destructor.
    Call clear().
    */
    ~BlockSet();

    /** Return a copy of this block set.
    Fragments and blocks are copied, sequences are not copied,
    sequence list is copied.
    Connections between the fragments
    (\ref Fragment::prev() "prev", \ref Fragment::next() "next")
    are rebuild with connect_fragments().
    \todo Preserve fragment connections from source block set.
    \see Block::clone()
    */
    BlockSetPtr clone() const;

    /** Add sequence.
    All the sequences, used by blocks, must be added.
    The sequence shared pointer is guaranteed to be kept until the set exists.
    */
    void add_sequence(SequencePtr seq);

    /** Add block.
    The same block can't be added twice.
    */
    void insert(BlockPtr block);

    /** Remove fragment.
    The block is deleted.
    */
    void erase(BlockPtr block);

    /** Return the number of blocks */
    size_t size() const;

    /** Return if there is no blocks */
    bool empty() const;

    /** Return if has the block */
    bool has(BlockPtr block) const;

    /** Remove all blocks.
    Removed blocks are deleted.
    */
    void clear();

    /** Get some block if any or an empty pointer */
    BlockPtr front() const;

    /** Return iterator to beginning */
    iterator begin();

    /** Return constant iterator to beginning */
    const_iterator begin() const;

    /** Return iterator to end */
    iterator end();

    /** Return constant iterator to end */
    const_iterator end() const;

    /** Connect all the fragments (prev-next) */
    void connect_fragments();

    /** Filter out fragments and blocks.
    \see Block::filter()
    */
    void filter(int min_fragment_length = 100, int min_block_size = 2);

    /** Merge neighbor blocks.
    \param join_approver Object confirming join.
        Value 0 means always approving one.
    */
    void join(JoinApprover* join_approver = 0);

    /** Expand all blocks (starting from blocks of large number of fragments).
    \see Block::expand()
    */
    void expand_blocks(PairAligner* aligner = 0, int batch = 100, int ori = 0,
                       int max_overlap = 0);

    /** Return if there are blocks which have overlaping fragments.
    \warning Fragments must be \ref BlockSet::connect_fragments "connected"
    */
    bool overlaps() const;

    /** Resolve overlaping fragments.
    If some blocks from the block set have overlaping fragments
    these two blocks are replaced with one higher (and narrower)
    block and several remainder blocks.

    Anyway, applying this method guarantees that no blocks of the block set
    have overlaping fragments.

    Since resolve_overlaps() can split blocks,
    applying \ref join "join(0)" is recommended.

    \verbatim
    Input:
        Block 1:
            seq1: ---xxxx----
            seq2: ---xxxx----
        Block 2
            seq2: -----xxxx--
            seq3: -----xxxx--
            seq4: -----xxxx--

    Output of resolve_overlaps:
        Block 1:
            seq1: ---xx------
            seq2: ---xx------
        Block 2
            seq2: -------xx--
            seq3: -------xx--
            seq4: -------xx--
        Block 3:
            seq1: -----xx----
            seq2: -----xx----
            seq3: -----xx----
            seq4: -----xx----
    \endverbatim
    \warning Fragments must be \ref BlockSet::connect_fragments "connected"
       for this to work correctly.
    */
    void resolve_overlaps();

    /** Expand all blocks by fragments.
    Return \p true is something was added.
    \see Block::expand_by_fragments().
    */
    bool expand_blocks_by_fragments(PairAligner* aligner = 0, int batch = 100);

    /** Return new block set of blocks of nucleotides, not included in this set.
    From sequences involved in this block set, nucleotides are selected,
    not included in this block set. They are grouped into fragments.
    Each fragment is inserted into one block.
    These blocks are inserted into resulting block set.
    \warning Fragments must be \ref BlockSet::connect_fragments "connected"
       for this to work correctly.
    */
    BlockSetPtr rest() const;

    /** Apply several methods to convert the block set to pangenome.
    This method could be applied to block set produced by AnchorFinder.
    */
    void make_pangenome();

    /** Add options to options description */
    static void add_output_options(po::options_description& desc);

    /** Write all blocks to file(s) or std::cout (depends on vm) */
    void make_output(const po::variables_map& vm);

private:
    Impl blocks_;
    std::vector<SequencePtr> seqs_;

    friend class BlockSetFastaReader;
};

/** Streaming operator.
\note Sequence list must be pre-added using BlockSet::add_sequence.
*/
std::istream& operator>>(std::istream& i, BlockSet& block_set);

/** Streaming operator */
std::ostream& operator<<(std::ostream& o, const BlockSet& block_set);

}

#endif

