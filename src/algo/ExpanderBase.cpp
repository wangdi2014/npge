/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <climits>

#include "ExpanderBase.hpp"
#include "Fragment.hpp"
#include "throw_assert.hpp"

namespace bloomrepeats {

ExpanderBase::ExpanderBase(int batch):
    batch_(batch)
{ }

bool ExpanderBase::aligned(const Fragment& f1, const Fragment& f2) const {
    int f1_last = -1, f2_last = -1;
    BOOST_ASSERT(f1.length() <= INT_MAX);
    BOOST_ASSERT(f2.length() <= INT_MAX);
    while (f1_last < int(f1.length()) - 1 &&
            f2_last < int(f2.length()) - 1) {
        int sub_this_last, sub_other_last;
        int this_min = std::min(int(f1.length()) - 1, f1_last + 1);
        int this_max = std::min(int(f1.length()) - 1, f1_last + batch());
        int other_min = std::min(int(f2.length()) - 1, f2_last + 1);
        int other_max = std::min(int(f2.length()) - 1, f2_last + batch());
        if (!aligner().aligned(f1.substr(this_min, this_max),
                               f2.substr(other_min, other_max),
                               &sub_this_last, &sub_other_last)) {
            return false;
        }
        if (sub_this_last == -1 || sub_other_last == -1) {
            return false;
        }
        f1_last += sub_this_last + 1;
        f2_last += sub_other_last + 1;
    }
    BOOST_ASSERT(f1_last < int(f1.length()) && f2_last < int(f2.length()));
    return aligner().aligned(f1.substr(f1_last, f1.length() - 1),
                             f2.substr(f2_last, f2.length() - 1));
}

}

