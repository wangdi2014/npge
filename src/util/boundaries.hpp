/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_BOUNDARIES_HPP_
#define BR_BOUNDARIES_HPP_

#include <vector>

namespace bloomrepeats {

/** Vector of positions.
Elements of the vector may be Fragment::min_pos() or Fragment::max_pos() + 1.
*/
typedef std::vector<size_t> Boundaries;

/** Return average value of the vector */
size_t avg_element(const Boundaries& boundaries);

/** Return value of the nearest element to the position */
size_t nearest_element(const Boundaries& boundaries, size_t pos);

/** Sort the vector and merge too close elements together */
void select_boundaries(Boundaries& boundaries, int min_distance);

}

#endif

