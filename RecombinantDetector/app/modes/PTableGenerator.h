#ifndef PTableGenerator_H
#define	PTableGenerator_H

#include <cassert>
#include "Run.h"

class PTableGenerator : public Run {
public:
    PTableGenerator(const PTableGenerator& orig) = delete;

    PTableGenerator& operator=(const PTableGenerator& rhs) = delete;

    explicit PTableGenerator(int argc, char** argv);

    ~PTableGenerator() override = default;

    bool isLogFileSupported() const override {
        return false;
    };

    Mode getMode() const override {
        return Mode::GeneratePTable;
    };

    void parseCmdLine() override;

    void perform() override;


private:
    string pTableFilePath;
    
    int pTableSize;
};

#endif	/* PTableGenerator_H */

