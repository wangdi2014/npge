/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_BLOCK_HPP_
#define BR_BLOCK_HPP_

#include <set>
#include <boost/enable_shared_from_this.hpp>

#include "global.hpp"

namespace bloomrepeats {

class Block : public boost::enable_shared_from_this<Block> {
public:
    typedef std::set<FragmentPtr> Impl;

    Block();

    void insert(FragmentPtr fragment);

    void erase(FragmentPtr fragment);

    size_t size() const;

    bool empty() const;

    bool has(FragmentPtr fragment) const;

    void clear();

    FragmentPtr front() const;

    Impl::iterator begin();

    Impl::const_iterator begin() const;

    Impl::iterator end();

    Impl::const_iterator end() const;

private:
    std::set<FragmentPtr> fragments_;
};

}

#endif

