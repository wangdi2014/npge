/*
 * bloomrepeats, Find genomic repeats, using Bloom filter based prefiltration
 * Copyright (C) 2012 Boris Nagaev
 *
 * See the LICENSE file for terms of use.
 */

#include "BlastFinder.hpp"
#include "Output.hpp"
#include "BlastRunner.hpp"
#include "ImportBlastHits.hpp"
#include "FileRemover.hpp"
#include "temp_file.hpp"
#include "name_to_stream.hpp"

namespace bloomrepeats {

BlastFinder::BlastFinder() {
    std::string consensuses = temp_file();
    std::string hits = temp_file();
    add(new Output, "--dump-seq:=1 --dump-block:=0 --out-file:="
            + consensuses);
    add(new BlastRunner, "--in-consensus:=" + consensuses +
        " --out-hits:=" + hits);
    add(new ImportBlastHits, "other=target --blast-hits:=" + hits);
    add(new FileRemover, "--filename:=" + consensuses);
    add(new FileRemover, "--filename:=" + hits);
}

}
