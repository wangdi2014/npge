/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BR_CONNECTOR_HPP_
#define BR_CONNECTOR_HPP_

#include "Processor.hpp"

namespace bloomrepeats {

/** Connect all the fragments (prev-next) */
class Connector : public Processor {
protected:
    /** Apply the action */
    bool run_impl() const;
};

}

#endif

