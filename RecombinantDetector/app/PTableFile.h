#pragma once
#ifndef PTABLEFILE_H
#define PTABLEFILE_H

#include <cassert>
#include <string>
#include "PTable.h"

class PTableFile {
public:

    enum ReadResult {
        INVALID_FILE /** The file is invalid or does not exist. */,
        WRONG_ARCH /** The P-value table file cannot be used on the current system architecture. */,
        CANCELLED /** The reading process is cancelled by the user. */,
        FILE_CORRUPT /** The file is corrupt. */,
        SUCCESS /** The P-value table has been loaded into RAM successfully. */
    };

    explicit PTableFile(std::string newFilePath) : filePath(newFilePath) {
    }

    ~PTableFile() {
    }

    // Get the path to the file from which the P-table is loaded
    const std::string getFilePath() const {
        return filePath;
    }

    const bool exists();
    const bool save(const PTable& pTable);

    const ReadResult tryLoadInto(PTable& pTable);
    static ReadResult Load(PTable& pTable);

private:

    /**
     * This string will be written at the beginning of every P-value table files.
     * It is helpful to test if a binary file is a P-value table quickly.
     */
    static const char FILE_MARKER[];

    PTableFile(const PTableFile& orig) : filePath(orig.filePath) {
        assert(false); // should never reach here
    }

    PTableFile& operator=(const PTableFile& rhs) {
        if (this != &rhs) {
            assert(false); // should never reach here
        }
        return *this;
    }

    std::string filePath;
};

#endif	/* PTABLEFILE_H */
