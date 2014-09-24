/*
 * NPG-explorer, Nucleotide PanGenome explorer
 * Copyright (C) 2014 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NPGE_STATS_HPP_
#define NPGE_STATS_HPP_

#include "Processor.hpp"
#include "FileWriter.hpp"

namespace npge {

/** Print human readable summary and statistics about blockset */
class Stats : public Processor {
public:
    /** Constructor */
    Stats();

    /** Access file writer */
    FileWriter& file_writer() {
        return file_writer_;
    }

protected:
    void run_impl() const;

    const char* name_impl() const;

private:
    FileWriter file_writer_;
};

}

#endif

