/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h> // for uint32_t
#include <climits>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

#include "Block.hpp"
#include "Fragment.hpp"
#include "PairAligner.hpp"
#include "Joiner.hpp"

namespace bloomrepeats {

struct Srander {
    Srander() {
        std::srand(time(NULL));
    }
} srander;

const int BLOCK_RAND_NAME_SIZE = 8;
const char* const BLOCK_RAND_NAME_ABC = "0123456789abcdef";
const int BLOCK_RAND_NAME_ABC_SIZE = 16;

Block::Block():
    name_(BLOCK_RAND_NAME_SIZE, '0')
{ }

Block::Block(const std::string& name) {
    set_name(name);
}

Block::~Block() {
    clear();
}

class BlockTag;

typedef boost::singleton_pool<BlockTag, sizeof(Block)> BlockPool;

void* Block::operator new(size_t /* x */) {
    return BlockPool::malloc();
}

void Block::operator delete(void* ptr) {
    BlockPool::free(ptr);
}

Block* Block::clone() const {
    Block* result = new Block(name());
    BOOST_FOREACH (Fragment* f, *this) {
        result->insert(new Fragment(*f));
    }
    return result;
}

void Block::insert(Fragment* fragment) {
    fragments_.push_back(fragment);
    fragment->set_block(this);
}

void Block::erase(Fragment* fragment) {
    Impl::iterator it = std::find(begin(), end(), fragment);
    BOOST_ASSERT(it != end());
    fragments_.erase(it);
    if (fragment->block_raw_ptr()) {
        fragment->set_block(0);
        delete fragment;
    }
}

size_t Block::size() const {
    return fragments_.size();
}

bool Block::empty() const {
    return fragments_.empty();
}

bool Block::has(Fragment* fragment) const {
    return std::find(begin(), end(), fragment) != end();
}

void Block::clear() {
    BOOST_FOREACH (Fragment* fragment, *this) {
        if (fragment->block_raw_ptr()) {
            fragment->set_block(0);
            delete fragment;
        }
    }
    fragments_.clear();
}

Fragment* Block::front() const {
    return empty() ? 0 : *(begin());
}

Block::Impl::iterator Block::begin() {
    return fragments_.begin();
}

Block::Impl::const_iterator Block::begin() const {
    return fragments_.begin();
}

Block::Impl::iterator Block::end() {
    return fragments_.end();
}

Block::Impl::const_iterator Block::end() const {
    return fragments_.end();
}

static struct FragmentCompareLength {
    bool operator()(const Fragment* f1, const Fragment* f2) const {
        return f1->length() < f2->length();
    }
} fcl;

float Block::identity() const {
    size_t total = (*std::max_element(begin(), end(), fcl))->length();
    size_t equal = 0;
    size_t min_length = (*std::min_element(begin(), end(), fcl))->length();
    for (size_t pos = 0; pos < min_length; pos++) {
        char c = 0;
        BOOST_FOREACH (Fragment* f, *this) {
            if (c == 0) {
                c = f->at(pos);
            } else if (c != f->at(pos)) {
                c = -1;
                break;
            }
        }
        if (c != -1) {
            equal += 1;
        }
    }
    return float(equal) / float(total);
}

int Block::match(Block* one, Block* another) {
    if (one->size() != another->size()) {
        return 0;
    }
    bool all_match = true;
    bool all_match_inversed = true;
    typedef std::map<int, int> OriCount;
    typedef std::map<Sequence*, OriCount> Seq2Ori;
    Seq2Ori seq2ori, seq2ori_other;
    BOOST_FOREACH (Fragment* fragment, *one) {
        seq2ori[fragment->seq()][fragment->ori()] += 1;
    }
    BOOST_FOREACH (Fragment* fragment, *another) {
        seq2ori_other[fragment->seq()][fragment->ori()] += 1;
    }
    BOOST_FOREACH (Seq2Ori::value_type& seq_and_ori, seq2ori) {
        Sequence* seq = seq_and_ori.first;
        OriCount& ori_count = seq_and_ori.second;
        Seq2Ori::iterator it = seq2ori_other.find(seq);
        if (it == seq2ori_other.end()) {
            return 0;
        }
        OriCount& ori_count_other = it->second;
        for (int ori = -1; ori <= 1; ori += 2) {
            if (ori_count[ori] != ori_count_other[ori]) {
                all_match = false;
            }
            if (ori_count[ori] != ori_count_other[-ori]) {
                all_match_inversed = false;
            }
        }
        if (!all_match && !all_match_inversed) {
            return 0;
        }
    }
    BOOST_ASSERT(all_match || all_match_inversed);
    return all_match ? 1 : -1;
}

void Block::inverse() {
    BOOST_FOREACH (Fragment* fragment, *this) {
        fragment->inverse();
    }
}

void Block::patch(const FragmentDiff& diff) {
    BOOST_FOREACH (Fragment* fragment, *this) {
        fragment->patch(diff);
    }
}

Block* Block::split(size_t new_length) {
    Block* result = new Block();
    BOOST_FOREACH (Fragment* fragment, *this) {
        Fragment* new_fragment = fragment->split(new_length);
        if (new_fragment) {
            result->insert(new_fragment);
        }
    }
    return result;
}

void Block::find_place() {
    BOOST_FOREACH (Fragment* fragment, *this) {
        fragment->find_place();
    }
}

int Block::max_shift_end(int max_overlap) const {
    int result = INT_MAX;
    BOOST_FOREACH (Fragment* f, *this) {
        result = std::min(result, f->max_shift_end(max_overlap));
    }
    return result;
}

size_t Block::common_positions(const Fragment& fragment) {
    size_t result = 0;
    BOOST_FOREACH (Fragment* f, *this) {
        result += f->common_positions(fragment);
    }
    return result;
}

void Block::merge(Block* other) {
    typedef std::map<Fragment, Fragment*> F2F;
    F2F f2f;
    std::vector<Fragment*> this_copy(begin(), end());
    BOOST_FOREACH (Fragment* f, this_copy) {
        if (f2f.find(*f) != f2f.end()) {
            delete f;
        } else {
            f2f[*f] = f;
        }
    }
    fragments_.clear();
    bool inverse_needed = false;
    BOOST_FOREACH (Fragment* f, *other) {
        f->inverse();
        if (f2f.find(*f) != f2f.end()) {
            inverse_needed = true;
        }
        f->inverse();
    }
    if (inverse_needed) {
        other->inverse();
    }
    std::vector<Fragment*> other_copy(other->begin(), other->end());
    BOOST_FOREACH (Fragment* f, other_copy) {
        if (f2f.find(*f) == f2f.end()) {
            f2f[*f] = f;
        } else {
            delete f;
        }
    }
    other->fragments_.clear();
    BOOST_FOREACH (F2F::value_type& f_and_ptr, f2f) {
        Fragment* f = f_and_ptr.second;
        insert(f);
    }
}

void Block::set_name(const std::string& name) {
    BOOST_ASSERT(name.length() >= 1 && name.length() <= 40);
#ifndef NDEBUG
    BOOST_FOREACH (char c, name) {
        BOOST_ASSERT(isalnum(c));
    }
#endif
    name_ = name;
}

void Block::set_random_name() {
    name_.resize(BLOCK_RAND_NAME_SIZE);
    for (size_t i = 0; i < BLOCK_RAND_NAME_SIZE; i++) {
        int r = rand() / (RAND_MAX / BLOCK_RAND_NAME_ABC_SIZE + 1);
        name_[i] = BLOCK_RAND_NAME_ABC[r];
    }
}

void Block::set_name_from_fragments() {
    std::vector<std::string> fragment_ids;
    BOOST_FOREACH (Fragment* f, *this) {
        fragment_ids.push_back(f->id());
    }
    std::sort(fragment_ids.begin(), fragment_ids.end());
    std::string joint = boost::algorithm::join(fragment_ids, " ");
    const int LOOP_SIZE = sizeof(uint32_t) * 2; // 2 = for * and for ^
    int new_size = ((joint.size() + LOOP_SIZE - 1) / LOOP_SIZE) * LOOP_SIZE;
    joint.resize(new_size, ' ');
    const uint32_t* value = reinterpret_cast<const uint32_t*>(joint.c_str());
    int loops = joint.size() / LOOP_SIZE;
    uint32_t a = 1;
    for (int i = 0; i < loops; i++) {
        a *= value[2 * i];
        a ^= value[2 * i + 1];
    }
    name_.resize(8);
    for (int byte_index = 0; byte_index < 4; byte_index++) {
        int byte = 0xFF & (a >> (8 * (3 - byte_index)));
        name_[byte_index * 2] = BLOCK_RAND_NAME_ABC[byte >> 4];
        name_[byte_index * 2 + 1] = BLOCK_RAND_NAME_ABC[byte & 0x0F];
    }
}

static struct FragmentCompareId {
    bool operator()(const Fragment* f1, const Fragment* f2) const {
        return f1->id() < f2->id();
    }
} fci;

std::ostream& operator<<(std::ostream& o, const Block& b) {
    std::vector<Fragment*> fragments(b.begin(), b.end());
    std::sort(fragments.begin(), fragments.end(), fci);
    BOOST_FOREACH (Fragment* f, fragments) {
        o << *f;
    }
    return o;
}

}

