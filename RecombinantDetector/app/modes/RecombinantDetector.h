#ifndef RecombinantDetector_H
#define	RecombinantDetector_H

#include <cassert>
#include <string>
#include "Run.h"
#include "../FastaReader.h"
#include "../PTableFile.h"
#include "../../core/Triplet.h"
#include "../../core/TripletPool.h"
#include "../../core/AlignmentDescriptor.h"


class RecombinantDetector : public Run {
public:
    explicit RecombinantDetector(int argc, char** argv);

    virtual ~RecombinantDetector();

    virtual bool isLogFileSupported() const {
        return true;
    };
    
    virtual Mode getMode() const {
        return Mode::RecombinantDetection;
    };
    
    virtual void parseCmdLine();
    
    virtual void perform();


private:
    RecombinantDetector(const RecombinantDetector& orig) = delete;

    RecombinantDetector& operator=(const RecombinantDetector& rhs) = delete;

    void dataInfo();
    void setup();
    void analyze();
    void showProgress(double currentLoop, bool isFinish, std::vector<size_t>& numsRecombinantTriplets, std::vector<double>& minPvals, std::vector<size_t>& numsSkippedByHeader) const;
    void displayResult();

private:
    bool m_readFromDir = false;
    bool m_tryToGetDataFromSequenceName = false;

    TripletPool m_tripletPool;
    
    AlignmentDescriptor m_alignmentDescriptor;

    PTableFile* m_pTableFile;
    
    double m_minPVal;

    size_t m_numTripletsForStatCorrection;

    size_t m_numRecombinantTriplets;
    size_t m_numComputedExactly;
    size_t m_numApproximated;
    size_t m_numSkipped;
    size_t m_numTripletsSkippedByTime;

    TextFile* m_fileSkippedTriplets;
    TextFile* m_fileRecombinants;
    TextFile* m_fileLongRecombinants;
};

#endif	/* RecombinantDetector_H */

