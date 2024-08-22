#include "PTableGenerator.h"

PTableGenerator::PTableGenerator(int argc, char** argv) : Run(argc, argv) {
    App::instance().startProgram("Generate P-Value Table");
}

void PTableGenerator::parseCmdLine() {
    if (getRunArgsNum() < 2) {
        App::instance() << "Not enough parameter to generate P-value table.\n";
        App::instance().showError(true, true);
    }

    pTableFilePath = m_argVector[2];
    pTableSize = 0;
    pTableSize = atoi(m_argVector[3].c_str());

    if (pTableSize < 2) {
        App::instance() << "Invalid table size.\n";
        App::instance().showError(true, true);
    }

    TextFile testFile(pTableFilePath);
    if (testFile.exists()) {
        testFile.removeFile();
    }
}

void PTableGenerator::perform() {
    PTable::instance().generateTable(pTableSize, pTableSize, pTableSize);

    if (PTable::instance().saveToFile(pTableFilePath)) {
        App::instance()
                << "The new P-value table has been stored into file: \"" << pTableFilePath << "\".\n";
        App::instance().showLog(true);
    } else {
        App::instance()
                << "The new P-value table cannot be stored into file: \"" << pTableFilePath << "\".\n"
                << "An error occurred during saving progress.\n";
        App::instance().showError(true, true);
    }
}