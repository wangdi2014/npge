/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <cctype>
#include <ostream>

#include "AlignmentRow.hpp"
#include "Fragment.hpp"
#include "throw_assert.hpp"

namespace bloomrepeats {

AlignmentRow::AlignmentRow(Fragment* fragment):
    length_(0), fragment_(fragment)
{ }

void AlignmentRow::grow(const std::string& alignment_string) {
    int align_pos = length();
    int fragment_pos = nearest_in_fragment(align_pos) + 1; // -1 -> 0
    for (int i = 0; i < alignment_string.size(); i++) {
        if (isalpha(alignment_string[i])) {
            BOOST_ASSERT(tolower(fragment()->raw_at(fragment_pos)) ==
                         tolower(alignment_string[i]));
            bind(fragment_pos, align_pos);
            fragment_pos += 1;
        }
        align_pos += 1;
    }
    set_length(length() + alignment_string.length());
}

int AlignmentRow::nearest_in_fragment(int align_pos) const {
    // FIXME do smth with this
    for (int distance = 0; distance < length(); distance++) {
        for (int ori = -1; ori <= 1; ori += 2) {
            int new_align_pos = align_pos + ori * distance;
            if (map_to_fragment(new_align_pos) != -1) {
                return map_to_fragment(new_align_pos);
            }
        }
    }
    return -1;
}

void AlignmentRow::print_alignment_string(std::ostream& o) const {
    // TODO gap char ('.', '~', etc)
    for (int align_pos = 0; align_pos < length(); align_pos++) {
        int fragment_pos = map_to_fragment(align_pos);
        if (fragment_pos == -1) {
            o << '-';
        } else {
            o << fragment()->raw_at(fragment_pos);
        }
    }
}

MapAlignmentRow::MapAlignmentRow(Fragment* fragment,
                                 const std::string& alignment_string):
    AlignmentRow(fragment) {
    grow(alignment_string);
}

void MapAlignmentRow::bind(int fragment_pos, int align_pos) {
    fragment_to_alignment_[fragment_pos] = align_pos;
    alignment_to_fragment_[align_pos] = fragment_pos;
}

int MapAlignmentRow::map_to_alignment(int fragment_pos) const {
    Pos2Pos::const_iterator it2 = fragment_to_alignment_.find(fragment_pos);
    if (it2 == fragment_to_alignment_.end()) {
        return -1;
    } else {
        return it2->second;
    }
}

int MapAlignmentRow::map_to_fragment(int align_pos) const {
    Pos2Pos::const_iterator it2 = alignment_to_fragment_.find(align_pos);
    if (it2 == alignment_to_fragment_.end()) {
        return -1;
    } else {
        return it2->second;
    }
}

std::ostream& operator<<(std::ostream& o, const AlignmentRow& row) {
    o << '>';
    row.fragment()->print_header(o);
    o << std::endl;
    row.print_alignment_string(o);
    o << std::endl;
    return o;
}

}

