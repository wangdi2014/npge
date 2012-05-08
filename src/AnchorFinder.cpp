/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <set>
#include <map>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "AnchorFinder.hpp"
#include "Sequence.hpp"
#include "Fragment.hpp"
#include "Block.hpp"
#include "BloomFilter.hpp"
#include "make_hash.hpp"

namespace bloomrepeats {

AnchorFinder::AnchorFinder():
    anchor_size_(ANCHOR_SIZE),
    only_ori_(0),
    workers_(1) {
    set_palindromes_elimination(true);
}

void AnchorFinder::add_sequence(SequencePtr s) {
    seqs_.push_back(s);
}

typedef std::set<size_t> Possible;

const size_t HASH_MUL = 1484954565;

static void test_and_add(SequencePtr s, BloomFilter& filter, size_t anchor_size,
                         Possible& p, int ori_to_add, int only_ori,
                         boost::mutex* mutex) {
    bool prev[3] = {false, false, false};
    Fragment f(s);
    s->make_first_fragment(f, anchor_size, only_ori);
    while (only_ori ? s->next_fragment_keeping_ori(f) : s->next_fragment(f)) {
        bool add = only_ori || f.ori() == ori_to_add;
        if (add && filter.test_and_add(f.begin(), anchor_size, f.ori()) ||
                !add && filter.test(f.begin(), anchor_size, f.ori())) {
            if (!prev[f.ori() + 1]) {
                prev[f.ori() + 1] = true;
                if (mutex) {
                    mutex->lock();
                }
                p.insert(make_hash(HASH_MUL, f.begin(), anchor_size, f.ori()));
                if (mutex) {
                    mutex->unlock();
                }
            }
        } else {
            prev[f.ori() + 1] = false;
        }
    }
}

typedef std::map<std::string, BlockPtr> StrToBlock;

static void find_blocks(SequencePtr s, size_t anchor_size, const Possible& p,
                        StrToBlock& str_to_block, int only_ori,
                        boost::mutex* mutex) {
    Fragment f(s);
    s->make_first_fragment(f, anchor_size, only_ori);
    while (only_ori ? s->next_fragment_keeping_ori(f) : s->next_fragment(f)) {
        size_t hash = make_hash(HASH_MUL, f.begin(), anchor_size, f.ori());
        if (p.find(hash) != p.end()) {
            FragmentPtr fragment = boost::make_shared<Fragment>(f);
            std::string key = fragment->str();
            BlockPtr block;
            if (mutex) {
                mutex->lock();
            }
            if (str_to_block.find(key) == str_to_block.end()) {
                block = str_to_block[key] = boost::make_shared<Block>();
            } else {
                block = str_to_block[key];
            }
            block->insert(fragment);
            if (mutex) {
                mutex->unlock();
            }
        }
    }
}

typedef boost::function<void()> Task;
typedef std::vector<Task> Tasks;

void process_some_seqs(Tasks& tasks, boost::mutex* mutex) {
    while (true) {
        Task task;
        if (mutex) {
            mutex->lock();
        }
        if (!tasks.empty()) {
            task = tasks.back();
            tasks.pop_back();
        }
        if (mutex) {
            mutex->unlock();
        }
        if (!task.empty()) {
            task();
        } else {
            break;
        }
    }
}

typedef boost::thread Thread;
typedef boost::shared_ptr<Thread> ThreadPtr;
typedef std::vector<ThreadPtr> Threads;

static void do_tasks(Tasks& tasks, int workers, boost::mutex* mutex) {
    Threads threads;
    for (int i = 1; i < workers; i++) {
        threads.push_back(boost::make_shared<boost::thread>(
                              boost::bind(process_some_seqs, boost::ref(tasks),
                                          mutex)));
    }
    process_some_seqs(tasks, mutex);
    BOOST_FOREACH (ThreadPtr thread, threads) {
        thread->join();
    }
}

void AnchorFinder::run() {
    boost::mutex* mutex = workers_ == 1 ? 0 : new boost::mutex();
    if (!anchor_handler_) {
        return;
    }
    size_t length_sum = 0;
    BOOST_FOREACH (SequencePtr s, seqs_) {
        length_sum += s->size();
    }
    if (std::log(length_sum) / std::log(4) > anchor_size_) {
        length_sum = std::pow(4, anchor_size_);
    }
    float error_prob = 1.0 / length_sum;
    std::set<size_t> possible_anchors;
    {
        BloomFilter filter(length_sum, error_prob);
        Tasks tasks;
        BOOST_FOREACH (SequencePtr s, seqs_) {
            tasks.push_back(
                boost::bind(test_and_add, s, boost::ref(filter),
                            anchor_size_, boost::ref(possible_anchors),
                            add_ori_, only_ori_, mutex));
        }
        do_tasks(tasks, workers_, mutex);
    }
    StrToBlock str_to_block;
    Tasks tasks;
    BOOST_FOREACH (SequencePtr s, seqs_) {
        tasks.push_back(
            boost::bind(find_blocks, s, anchor_size_,
                        boost::ref(possible_anchors),
                        boost::ref(str_to_block), only_ori_, mutex));
    }
    do_tasks(tasks, workers_, mutex);
    BOOST_FOREACH (const StrToBlock::value_type& key_and_block, str_to_block) {
        BlockPtr block = key_and_block.second;
        if (block->size() >= 2) {
            anchor_handler_(block);
        }
    }
    delete mutex;
}

bool AnchorFinder::palindromes_elimination() const {
    return add_ori_ == -Sequence::FIRST_ORI;
}

void AnchorFinder::set_palindromes_elimination(bool eliminate) {
    add_ori_ = eliminate ? -Sequence::FIRST_ORI : Sequence::FIRST_ORI;
}

}

