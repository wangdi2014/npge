/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2012-2016 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_BLOCK_HASH_HPP_
#define NPGE_BLOCK_HASH_HPP_

#include "global.hpp"

namespace npge {

/** Return hash of block.
Fragment::id() (i.e., sequence names, fragment positions and ori)
affects hash value.
Alignment and order of fragments does not.
*/
hash_t block_hash(const Block* block);

/** Return hash of block.
Hashes of blocks of blockset are XOR'ed.
Blocks of <=1 fragment are skipped.
*/
hash_t blockset_hash(const BlockSet& block_set,
                     int workers = 1);

/** Return block id (<size>x<length>) */
std::string block_id(const Block* block);

/** Return if block has fragments from same genome */
bool has_repeats(const Block* block);

/** Return if block has no repeats and its site equals to genomes */
bool is_exact_stem(const Block* block, int genomes);

/** Return number of genomes occupied by blockset */
int genomes_number(const BlockSet& block_set);

/** Return block name.
Block name format:
 - first letter
   - r for repeat (>= 2 fragments on same genome)
   - s for exact stem (= 1 fragment on each genome)
   - h for other blocks of >=2 fragments
   - u for blocks of 1 fragment.
 - block_id
*/
std::string block_name(const Block* block, int genomes);

/** Set block name to block_name() */
void set_canonical_name(Block* block, int genomes);

/** Return if all fragments of block have alignment */
bool has_alignment(const Block* block);

/** Compare blocks by size, length and by min fragment */
bool block_less(const Block* a, const Block* b);

/** Compare blocks by size, length and by min fragment */
bool block_greater(const Block* a, const Block* b);

/** Check block rows.
All-or-none, same length.
*/
void test_block(const Block* block);

}

#endif

