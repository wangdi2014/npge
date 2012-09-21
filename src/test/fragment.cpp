/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include "Sequence.hpp"
#include "Fragment.hpp"
#include "PairAligner.hpp"

BOOST_AUTO_TEST_CASE (Fragment_main) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 9, 1);
    BOOST_REQUIRE(f1.length() == 10);
    BOOST_CHECK(f1.str() == "tggtccgaga");
    Fragment f2(s1, 0, 9, -1);
    BOOST_REQUIRE(f2.length() == 10);
    BOOST_CHECK(f2.str() == "tctcggacca");
    BOOST_CHECK(f1.substr(1, 1) == "g");
    BOOST_CHECK(f1.substr(1, 2) == "gg");
    BOOST_CHECK(f1.substr(1, -1) == "ggtccgaga");
    BOOST_CHECK(f1.substr(-2, -1) == "ga");
    BOOST_CHECK(f2.substr(1, 1) == "c");
    BOOST_CHECK(f2.substr(1, 2) == "ct");
    BOOST_CHECK(f2.substr(1, -1) == "ctcggacca");
    BOOST_CHECK(f2.substr(-2, -1) == "ca");
}

BOOST_AUTO_TEST_CASE (Fragment_subfragment) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    // ----------------------------------------------------0123456789
    Fragment f1(s1, 0, 9, 1);
    Fragment f2(s1, 0, 9, -1);
    FragmentPtr tmp;
    //
    tmp = f1.subfragment(1, 1);
    BOOST_CHECK(tmp->str() == "g");
    delete tmp;
    //
    tmp = f1.subfragment(0, 5);
    BOOST_CHECK(tmp->str() == "tggtcc");
    delete tmp;
    //
    tmp = f1.subfragment(5, 0);
    BOOST_CHECK(tmp->str() == "ggacca");
    delete tmp;
    //
    tmp = f2.subfragment(1, 1);
    BOOST_CHECK(tmp->str() == "c");
    delete tmp;
    //
    tmp = f2.subfragment(0, 5);
    BOOST_CHECK(tmp->str() == "tctcgg");
    delete tmp;
    //
    tmp = f2.subfragment(5, 0);
    BOOST_CHECK(tmp->str() == "ccgaga");
    delete tmp;
}

BOOST_AUTO_TEST_CASE (Fragment_assign) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 9, 1);
    Fragment f2(f1);
    f2 = f2;
    Fragment f3;
    f3 = f2;
    BOOST_CHECK(f1 == Fragment(s1, 0, 9, 1));
    BOOST_CHECK(f2 == Fragment(s1, 0, 9, 1));
    BOOST_CHECK(f3 == Fragment(s1, 0, 9, 1));
}

BOOST_AUTO_TEST_CASE (Fragment_expand) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("TGGTCCGAGAtgcgggcc");
    Fragment f1(s1, 0, 9, 1);
    BOOST_REQUIRE(f1.length() == 10);
    BOOST_CHECK(f1.max_shift_end() == 8);
    f1.shift_end();
    BOOST_CHECK(f1.valid());
    BOOST_CHECK(f1.length() == 11);
    BOOST_CHECK(f1.max_shift_end() == 7);
    f1.inverse();
    f1.shift_end();
    BOOST_CHECK(!f1.valid());
    BOOST_CHECK(f1.max_shift_end() == -1);
    f1.shift_end(-1);
    BOOST_CHECK(f1.length() == 11);
    BOOST_CHECK(f1.valid());
    BOOST_CHECK(f1.max_shift_end() == 0);
    f1.shift_end(-1);
    BOOST_CHECK(f1.length() == 10);
    BOOST_CHECK(f1.min_pos() == 1);
    BOOST_CHECK(f1.max_shift_end() == 1);
    //
    f1.shift_end(f1.max_shift_end());
    f1.inverse();
    f1.shift_end(f1.max_shift_end());
    BOOST_CHECK(f1.valid());
    BOOST_CHECK(f1.min_pos() == 0);
    BOOST_CHECK(f1.max_pos() == s1->size() - 1);
}

BOOST_AUTO_TEST_CASE (Fragment_max_shift_two_fragments) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("ggtGGTcCGAga");
    FragmentPtr f1 = Fragment::create_new(s1, 3, 5);
    FragmentPtr f2 = Fragment::create_new(s1, 7, 9);
    Fragment::connect(f1, f2);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ -1) == 6);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 0) == 1);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 1) == 2);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 4) == 5);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 100) == 6);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ -1) == 2);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ 0) == 2);
    f1->inverse();
    BOOST_CHECK(f1->max_shift_end(/* overlap */ -1) == 3);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 0) == 3);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ -1) == 2);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ 0) == 2);
    f2->inverse();
    BOOST_CHECK(f1->max_shift_end(/* overlap */ -1) == 3);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 0) == 3);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ -1) == 7);
    BOOST_CHECK(f2->max_shift_end(/* overlap */ 0) == 1);
    // if f2 is invalid (f2 < 0)
    f1->disconnect();
    Fragment::connect(f2, f1);
    f2->set_min_pos(-22);
    f2->set_max_pos(-20);
    BOOST_REQUIRE(!f2->valid());
    BOOST_CHECK(f1->max_shift_end(/* overlap */ -1) == 3);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 0) == 3);
    // if f2 is invalid (f2 > seq.length)
    f1->disconnect();
    Fragment::connect(f1, f2);
    f1->inverse();
    f2->set_min_pos(20);
    f2->set_max_pos(22);
    BOOST_REQUIRE(!f2->valid());
    BOOST_CHECK(f1->max_shift_end(/* overlap */ -1) == 6);
    BOOST_CHECK(f1->max_shift_end(/* overlap */ 0) == 6);
    delete f1;
    delete f2;
}

BOOST_AUTO_TEST_CASE (Fragment_equal) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 9, 1);
    Fragment f2(s1, 0, 9, 1);
    Fragment f3(s1, 0, 9, -1);
    BOOST_CHECK(f1 == f1);
    BOOST_CHECK(f1 == f2);
    BOOST_CHECK(f1 != f3);
}

BOOST_AUTO_TEST_CASE (Fragment_less) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    BOOST_CHECK(!(Fragment(s1, 0, 9, 1) < Fragment(s1, 0, 9, 1)));
    BOOST_CHECK(Fragment(s1, 0, 9, 1) < Fragment(s1, 2, 9, 1));
    BOOST_CHECK(Fragment(s1, 0, 9, 1) < Fragment(s1, 0, 10, 1));
    BOOST_CHECK(Fragment(s1, 0, 9, -1) < Fragment(s1, 0, 9, 1));
    BOOST_CHECK(Fragment(s1, 0, 9, -1) < Fragment(s2, 0, 9, 1) ||
                Fragment(s2, 0, 9, -1) < Fragment(s1, 0, 9, 1));
}

BOOST_AUTO_TEST_CASE (Fragment_raw_at) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    Fragment f1(s1, 5, 10, 1);
    Fragment f2(s1, 5, 10, -1);
    BOOST_CHECK(f1.raw_at(0) == 'c');
    BOOST_CHECK(f1.raw_at(1) == 'g');
    BOOST_CHECK(f1.raw_at(-1) == 'c');
    BOOST_CHECK(f1.raw_at(-2) == 't');
    BOOST_CHECK(f2.raw_at(0) == 'a');
    BOOST_CHECK(f2.raw_at(1) == 't');
    BOOST_CHECK(f2.raw_at(-1) == 'c');
    BOOST_CHECK(f2.raw_at(-2) == 'g');
}

BOOST_AUTO_TEST_CASE (Fragment_at) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    Fragment f1(s1, 5, 10, 1);
    Fragment f2(s1, 5, 10, -1);
    BOOST_CHECK(f1.at(0) == 'c');
    BOOST_CHECK(f1.at(1) == 'g');
    BOOST_CHECK(f1.at(-1) == 't');
    BOOST_CHECK(f1.at(-2) == 'a');
    BOOST_CHECK(f2.at(0) == 'a');
    BOOST_CHECK(f2.at(1) == 't');
    BOOST_CHECK(f2.at(-1) == 'g');
    BOOST_CHECK(f2.at(-2) == 'c');
}

BOOST_AUTO_TEST_CASE (Fragment_next) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f2);
    Fragment::connect(f2, f3);
    Fragment::connect(f3, f1);
    BOOST_REQUIRE(f1->next() == f2);
    BOOST_REQUIRE(f2->next() == f3);
    BOOST_REQUIRE(f3->next() == f1);
    BOOST_REQUIRE(f1->prev() == f3);
    BOOST_REQUIRE(f2->prev() == f1);
    BOOST_REQUIRE(f3->prev() == f2);
    f2->disconnect(/* connect_neighbors */ true);
    BOOST_CHECK(f1->next() == f3);
    BOOST_CHECK(f1->prev() == f3);
    BOOST_CHECK(f3->prev() == f1);
    BOOST_CHECK(f3->next() == f1);
    f1->disconnect(/* connect_neighbors */ false);
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(!f1->next());
    BOOST_CHECK(!f3->prev());
    BOOST_CHECK(!f3->next());
    delete f1;
    delete f2;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_dtor) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f2);
    Fragment::connect(f2, f3);
    Fragment::connect(f3, f1);
    BOOST_REQUIRE(f1->next() == f2);
    BOOST_REQUIRE(f2->next() == f3);
    BOOST_REQUIRE(f3->next() == f1);
    BOOST_REQUIRE(f1->prev() == f3);
    BOOST_REQUIRE(f2->prev() == f1);
    BOOST_REQUIRE(f3->prev() == f2);
    delete f2; // (= disconnect(true) )
    BOOST_CHECK(f1->next() == f3);
    BOOST_CHECK(f1->prev() == f3);
    BOOST_CHECK(f3->prev() == f1);
    BOOST_CHECK(f3->next() == f1);
    f1->disconnect(/* connect_neighbors */ false);
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(!f1->next());
    BOOST_CHECK(!f3->prev());
    BOOST_CHECK(!f3->next());
    delete f1;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_connect_ori) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    Fragment::connect(f1, f2);
    BOOST_REQUIRE(f1->next() == f2);
    BOOST_REQUIRE(!f2->next());
    BOOST_REQUIRE(!f1->prev());
    BOOST_REQUIRE(f2->prev() == f1);
    Fragment::connect(f1, f2, -1); // cycle
    BOOST_REQUIRE(f1->next() == f2);
    BOOST_REQUIRE(f2->next() == f1);
    BOOST_REQUIRE(f1->prev() == f2);
    BOOST_REQUIRE(f2->prev() == f1);
    delete f1;
    delete f2;
}

BOOST_AUTO_TEST_CASE (Fragment_rearrange_with) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tGGtcCGAGatgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f2, -1); // wrong order
    Fragment::connect(f2, f3, -1); // wrong order
    f1->rearrange_with(f3);
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(f2->next() == f3);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f3->prev() == f2);
    f1->rearrange_with(f2);
    BOOST_CHECK(f1->next() == f3);
    BOOST_CHECK(f2->next() == f1);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(f1->prev() == f2);
    BOOST_CHECK(!f2->prev());
    BOOST_CHECK(f3->prev() == f1);
    delete f1;
    delete f2;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_find_place) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tGGtcCGAGatgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f2, -1); // wrong order
    Fragment::connect(f2, f3, -1); // wrong order
    f1->find_place();
    f2->find_place();
    f3->find_place();
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(f2->next() == f3);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f3->prev() == f2);
    delete f1;
    delete f2;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_find_place_f) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tGGtcCGAGatgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f3);
    f2->find_place(f1);
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(f2->next() == f3);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f3->prev() == f2);
    f1->find_place(f2);
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(f2->next() == f3);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f3->prev() == f2);
    delete f1;
    delete f2;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_neighbor) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, 1);
    Fragment::connect(f1, f2);
    Fragment::connect(f2, f3);
    Fragment::connect(f3, f1);
    BOOST_CHECK(f1->neighbor(1) == f2);
    BOOST_CHECK(f1->neighbor(-1) == f3);
    BOOST_CHECK(f2->neighbor(1) == f3);
    BOOST_CHECK(f2->neighbor(-1) == f1);
    BOOST_CHECK(f3->neighbor(1) == f1);
    BOOST_CHECK(f3->neighbor(-1) == f2);
    //
    BOOST_CHECK(f1->logical_neighbor(1) == f2);
    BOOST_CHECK(f1->logical_neighbor(-1) == f3);
    BOOST_CHECK(f2->logical_neighbor(1) == f1);
    BOOST_CHECK(f2->logical_neighbor(-1) == f3);
    BOOST_CHECK(f3->logical_neighbor(1) == f1);
    BOOST_CHECK(f3->logical_neighbor(-1) == f2);
    //
    BOOST_CHECK(f1->is_neighbor(*f2));
    BOOST_CHECK(f2->is_neighbor(*f1));
    //
    BOOST_CHECK(f2->another_neighbor(*f1) == f3);
    BOOST_CHECK(f3->another_neighbor(*f2) == f1);
    delete f1;
    delete f2;
    delete f3;
}

BOOST_AUTO_TEST_CASE (Fragment_common_positions) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    Fragment f4(s2, 6, 8, -1);
    BOOST_CHECK(f1.common_positions(f2) == 1);
    BOOST_CHECK(f2.common_positions(f1) == 1);
    BOOST_CHECK(f2.common_positions(f3) == 3);
    BOOST_CHECK(f3.common_positions(f2) == 3);
    BOOST_CHECK(f1.common_positions(f3) == 0);
    BOOST_CHECK(f3.common_positions(f1) == 0);
    BOOST_CHECK(f3.common_positions(f4) == 0);
}

BOOST_AUTO_TEST_CASE (Fragment_dist_to) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 6, -1);
    Fragment f3(s1, 7, 8, -1);
    BOOST_CHECK(f1.dist_to(f2) == 0);
    BOOST_CHECK(f2.dist_to(f1) == 0);
    BOOST_CHECK(f1.dist_to(f3) == 1);
    BOOST_CHECK(f3.dist_to(f1) == 1);
    BOOST_CHECK(f2.dist_to(f3) == 0);
    BOOST_CHECK(f3.dist_to(f2) == 0);
}

BOOST_AUTO_TEST_CASE (Fragment_common_fragment) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    Fragment f4(s2, 6, 8, -1);
    BOOST_CHECK(f1.common_fragment(f2) == Fragment(s1, 5, 5));
    BOOST_CHECK(f2.common_fragment(f1) == Fragment(s1, 5, 5, -1));
    BOOST_CHECK(f2.common_fragment(f3) == Fragment(s1, 6, 8, -1));
    BOOST_CHECK(f3.common_fragment(f2) == Fragment(s1, 6, 8, -1));
    BOOST_CHECK(!f1.common_fragment(f3).valid());
    BOOST_CHECK(!f3.common_fragment(f1).valid());
    BOOST_CHECK(!f3.common_fragment(f4).valid());
}

BOOST_AUTO_TEST_CASE (Fragment_is_subfragment) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    SequencePtr s2 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    Fragment f3a(s1, 5, 8, -1);
    Fragment f4(s2, 6, 8, -1);
    BOOST_CHECK(!f1.is_subfragment_of(f2));
    BOOST_CHECK(!f1.is_internal_subfragment_of(f2));
    BOOST_CHECK(f2.is_subfragment_of(f2));
    BOOST_CHECK(!f2.is_internal_subfragment_of(f2));
    BOOST_CHECK(!f2.is_subfragment_of(f1));
    BOOST_CHECK(!f2.is_internal_subfragment_of(f1));
    BOOST_CHECK(!f3.is_subfragment_of(f4));
    BOOST_CHECK(!f3.is_internal_subfragment_of(f4));
    BOOST_CHECK(f3.is_subfragment_of(f3a));
    BOOST_CHECK(!f3.is_internal_subfragment_of(f3a));
    BOOST_CHECK(f3.is_subfragment_of(f2));
    BOOST_CHECK(f3.is_internal_subfragment_of(f2));
}

BOOST_AUTO_TEST_CASE (Fragment_join) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtcCGAGATgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 1, 2, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 5, 6, 1);
    FragmentPtr f3 = Fragment::create_new(s1, 7, 8, -1);
    Fragment::connect(f1, f2);
    Fragment::connect(f2, f3);
    Fragment::connect(f3, f1);
    BOOST_CHECK(Fragment::can_join(f1, f2));
    BOOST_CHECK(Fragment::can_join(f2, f1));
    BOOST_CHECK(!Fragment::can_join(f1, f3));
    BOOST_CHECK(!Fragment::can_join(f2, f3));
    FragmentPtr f12 = Fragment::join(f1, f2);
    BOOST_CHECK(f12->ori() == 1);
    BOOST_CHECK(f12->seq() == s1.get());
    BOOST_CHECK(f12->min_pos() == 1);
    BOOST_CHECK(f12->max_pos() == 6);
    BOOST_CHECK(f12->is_neighbor(*f3));
    BOOST_CHECK(!f12->is_neighbor(*f1));
    BOOST_CHECK(!f12->is_neighbor(*f2));
    delete f1;
    delete f2;
    delete f3;
    delete f12;
}

BOOST_AUTO_TEST_CASE (Fragment_diff_patch) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f0(s1, 0, 5, 1);
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    f1.patch(f1.diff_to(f2));
    f3.patch(f3.diff_to(f2));
    BOOST_CHECK(f1 == f2);
    BOOST_CHECK(f3 == f2);
    f2.patch(f0.diff_to(f2));
    BOOST_CHECK(f2 == f0); // symmetrical
}

BOOST_AUTO_TEST_CASE (Fragment_exclude) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    f1.exclude(f3);
    BOOST_CHECK(f1 == Fragment(s1, 0, 5, 1));
    f2.exclude(f1);
    BOOST_CHECK(f2 == Fragment(s1, 6, 10, -1));
    f2.exclude(f3);
    BOOST_CHECK(f2 == Fragment(s1, 9, 10, -1));
    BOOST_CHECK(f2.valid());
    f2.exclude(Fragment(s1, 9, 10, -1));
    BOOST_CHECK(!f2.valid());
    BOOST_CHECK(f3.valid());
    f3.exclude(Fragment(s1, 2, 10, 1));
    BOOST_CHECK(!f3.valid());
}

BOOST_AUTO_TEST_CASE (Fragment_exclusion_diff) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    Fragment f1(s1, 0, 5, 1);
    Fragment f2(s1, 5, 10, -1);
    Fragment f3(s1, 6, 8, -1);
    f1.patch(f1.exclusion_diff(f3));
    BOOST_CHECK(f1 == Fragment(s1, 0, 5, 1));
    f2.patch(f2.exclusion_diff(f1));
    BOOST_CHECK(f2 == Fragment(s1, 6, 10, -1));
    f2.patch(f2.exclusion_diff(f3));
    BOOST_CHECK(f2 == Fragment(s1, 9, 10, -1));
    BOOST_CHECK(f2.valid());
    f2.patch(f2.exclusion_diff(Fragment(s1, 9, 10, -1)));
    BOOST_CHECK(!f2.valid());
    BOOST_CHECK(f3.valid());
    f3.patch(f3.exclusion_diff(Fragment(s1, 2, 10, 1)));
    BOOST_CHECK(!f3.valid());
}

BOOST_AUTO_TEST_CASE (Fragment_split) {
    using namespace bloomrepeats;
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccgagatgcgggcc");
    FragmentPtr f1 = Fragment::create_new(s1, 0, 10, 1);
    FragmentPtr f2 = Fragment::create_new(s1, 3, 5, -1);
    FragmentPtr f3 = Fragment::create_new(s1, 6, 8, -1);
    Fragment::connect(f1, f2);
    Fragment::connect(f2, f3);
    FragmentPtr f2a = f1->split(5);
    BOOST_REQUIRE(f1);
    BOOST_REQUIRE(f2a);
    BOOST_CHECK(*f1 == Fragment(s1, 0, 4, 1));
    BOOST_CHECK(*f2a == Fragment(s1, 5, 10, 1));
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->next() == f2a);
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f2a->next() == f3);
    BOOST_CHECK(f2a->prev() == f2);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(f3->prev() == f2a);
    //
    FragmentPtr f_null = f1->split(5);
    BOOST_CHECK(!f_null);
    BOOST_CHECK(*f1 == Fragment(s1, 0, 4, 1));
    BOOST_CHECK(f1->next() == f2);
    BOOST_CHECK(!f1->prev());
    BOOST_CHECK(f2->next() == f2a);
    BOOST_CHECK(f2->prev() == f1);
    BOOST_CHECK(f2a->next() == f3);
    BOOST_CHECK(f2a->prev() == f2);
    BOOST_CHECK(!f3->next());
    BOOST_CHECK(f3->prev() == f2a);
    //
    FragmentPtr f3a = f3->split(1);
    BOOST_REQUIRE(f3a);
    BOOST_CHECK(*f3 == Fragment(s1, 8, 8, -1));
    BOOST_CHECK(*f3a == Fragment(s1, 6, 7, -1));
    BOOST_CHECK(f3->prev() == f3a);
    BOOST_CHECK(f3a->prev() == f2a);
    delete f1;
    delete f2;
    delete f2a;
    delete f3;
    delete f3a;
}

BOOST_AUTO_TEST_CASE (Fragment_aligned) {
    using namespace bloomrepeats;
    PairAligner eq(0);
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccga|tggtccga");
    BOOST_CHECK(Fragment(s1, 0, 0).aligned(Fragment(s1, 8, 8), &eq));
    BOOST_CHECK(Fragment(s1, 0, 0).aligned(Fragment(s1, 3, 3), &eq));
    BOOST_CHECK(!Fragment(s1, 0, 0).aligned(Fragment(s1, 1, 1), &eq));
    BOOST_CHECK(!Fragment(s1, 0, 0).aligned(Fragment(s1, 1, 1), &eq, 1));
    BOOST_CHECK(Fragment(s1, 0, 0).aligned(Fragment(s1, 0, 0), &eq));
    BOOST_CHECK(Fragment(s1, 0, 0).aligned(Fragment(s1, 7, 7, -1), &eq));
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 15), &eq));
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 15), &eq, 3));
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 15), &eq, 1));
    BOOST_CHECK(!Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 14), &eq));
    PairAligner oe(1, 1, 1);
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 14), &oe));
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 14), &oe, 3));
    BOOST_CHECK(Fragment(s1, 0, 7).aligned(Fragment(s1, 8, 14), &oe, 1));
}

BOOST_AUTO_TEST_CASE (Fragment_id) {
    using namespace bloomrepeats;
    PairAligner eq(0);
    SequencePtr s1 = boost::make_shared<InMemorySequence>("tggtccga|tggtccga");
    BOOST_CHECK(Fragment(s1, 1, 2).id() == "_1_2");
    BOOST_CHECK(Fragment(s1, 1, 2, -1).id() == "_2_1");
    s1->set_name("seq");
    BOOST_CHECK(Fragment(s1, 1, 2, -1).id() == "seq_2_1");
}

