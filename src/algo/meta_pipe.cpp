/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include "meta_pipe.hpp"
#include "Pipe.hpp"
#include "Meta.hpp"
#include "throw_assert.hpp"

namespace bloomrepeats {

typedef std::vector<char> String;

std::string to_string(const String& k) {
    return std::string(k.begin(), k.end());
}

void set_k(Pipe* pipe, const String& k) {
    pipe->set_key(to_string(k));
}

void set_n(Pipe* pipe, const String& k) {
    pipe->set_name(to_string(k));
}

typedef boost::fusion::vector<String, String> TwoStrings;

void add_p(Pipe* pipe, const Meta* meta, const TwoStrings& processor) {
    std::string key = to_string(boost::fusion::at_c<0>(processor));
    std::string options = to_string(boost::fusion::at_c<1>(processor));
    BOOST_ASSERT_MSG(meta->has(key), "No such processor: " + key);
    ProcessorPtr p = meta->get(key);
    pipe->add(p, options);
}

template <typename Iterator>
bool parse_pipe(Iterator first, Iterator last,
                Pipe* pipe, const Meta* meta) {
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    using qi::bool_;
    using qi::char_;
    using qi::int_;
    using qi::lexeme;
    using qi::lit;
    using qi::phrase_parse;
    using ascii::space;
    bool r = phrase_parse(first, last,
    //  Begin grammar
    (
    lit("pipe") >> (+(char_ - ' ' - '{'))[boost::bind(set_k, pipe, _1)]
    >> '{' >> *(
        lit("name") >> lexeme['"' >> +(char_ - '"') >> '"']
            [boost::bind(set_n, pipe, _1)] >> ';'
        || lit("max_loops") >> int_
            [boost::bind(&Pipe::set_max_loops, pipe, _1)] >> ';'
        || lit("workers") >> int_
            [boost::bind(&Processor::set_workers, pipe, _1)] >> ';'
        || lit("no_options") >> bool_
            [boost::bind(&Processor::set_no_options, pipe, _1)] >> ';'
        || lit("timing") >> bool_
            [boost::bind(&Processor::set_timing, pipe, _1)] >> ';'
        || lit("add") >> ((+char_ - ' ' - ';') >> (+char_))
            [boost::bind(add_p, pipe, meta, _1)] >> ';'
     ) >> '}' >> ';'
    )
    ,
    //  End grammar
    space);
    return r && (first == last);
}

static Meta default_meta;

boost::shared_ptr<Pipe> create_pipe(const std::string& script,
        const Meta* meta) {
    if (meta == 0) {
        meta = &default_meta;
    }
    boost::shared_ptr<Pipe> result(new Pipe);
    bool ok = parse_pipe(script.begin(), script.end(), result.get(), meta);
    BOOST_ASSERT_MSG(ok, "Can't parse pipe description: " + script);
    return result;
}

}

