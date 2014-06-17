/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_META_HPP_
#define NPGE_META_HPP_

#include <string>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "global.hpp"
#include "AnyAs.hpp"

namespace npge {

/** Return processor by key */
class Meta {
public:
    /** Constructor.
    Add returners of all processors from algo/.
    */
    Meta();

    /** Destructor */
    virtual ~Meta();

    /** Return if a processor is associated with the key */
    bool has(const std::string& key) const;

    /** Return plain pointer to processor instance by key.
    If no processor were associated with the key,
    assertation exception is thrown.

    Processor.meta() of returned processor is set to this.
    */
    Processor* get_plain(const std::string& key) const;

    /** Return shared ptr to processor instance by key.
    \see get_plain()
    */
    SharedProcessor get(const std::string& key) const;

    /** Associate function returning processor.
    Key is taken as function()->key().
    */
    template<typename F>
    void set_returner(const F& function) {
        Processor* p = function();
        map_[get_key_and_delete(p)] = function;
    }

    /** Associate processor type with key.
    \see set_returner()
    */
    template<typename P>
    void set_processor() {
        set_returner(&Meta::new_processor<P>);
    }

    /** Return keys list */
    Strings keys() const;

    /** Return if no processor returners were set */
    bool empty() const;

    /** Remore all processor returners and options */
    void clear();

    /** Return empty processor which lives till meta object lives.
    This can be used to handle blocksets across different "run"s in script.
    */
    Processor* placeholder_processor() const {
        return placeholder_processor_;
    }

    /** Function returning AnyAs */
    typedef boost::function<AnyAs()> AnyReturner;

    /** Get global option */
    AnyAs get_opt(const std::string& key, const AnyAs& dflt = 0) const;

    /** Set global option */
    void set_opt(const std::string& key, const AnyAs& value);

    /** Set global option getter */
    void set_opt_func(const std::string& key, const AnyReturner& f);

    /** List global options */
    Strings opts() const;

    /** Remove global option */
    void remove_opt(const std::string& key);

private:
    typedef Processor* ProcessorPtr;
    typedef boost::function<ProcessorPtr()> ProcessorReturner;
    typedef std::map<std::string, ProcessorReturner> ReturnerMap;
    typedef std::map<std::string, AnyReturner> AnyMap;

    ReturnerMap map_;
    AnyMap opts_;
    Processor* placeholder_processor_;

    template<typename P>
    static P* new_processor() {
        return new P;
    }

    static std::string get_key_and_delete(const Processor* p);
};

}

#endif

