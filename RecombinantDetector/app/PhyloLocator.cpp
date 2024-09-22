#include <iostream>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <unordered_map>

#include "App.h"
#include "modes/Run.h"

#include "../core/PhyloNode.h"
#include "../core/PhyloTree.h"
#include "../utils/Utils.h"

struct Sequence;
class PhyloNode;
class PhyloTree;

int main(int argc, char** argv) {
    unsigned int seed = time(nullptr);
    srand(seed);

    if (argc >= 1) {
        App::instance().setExecutablePath(argv[0]);
    }

    Run* run = nullptr;

    try {
        // Detect run-m_mode and return a run-object
        run = Run::getRun(argc, argv);

        if (run == nullptr) {
            App::instance().startProgram("Cannot detect run mode.");
            App::instance().showPTableLink();
            App::instance().showLog(true);

        }
        else {
            // Here is the main flow of every analysis (of all kinds)
            run->parseCmdLine();
            run->initLogFile();
            run->perform();
        }

        if (run == nullptr) {
            App::instance().throwExitSignal(0);
        }

    }
    catch (App::ExitSignal) {
    }
    catch (const std::exception& ex) {
        App::instance() << ex.what() << std::endl;
        try {
            App::instance().showError(true, true);
        }
        catch (...) {
        }

    }
    catch (...) {
        App::instance()
            << "An unexplained exception occurred during the analysis.\n";
        try {
            App::instance().showError(true, true);
        }
        catch (...) {
        }
    }

    if (run != nullptr) {
        delete run;
        run = nullptr;
    }

    return 0;
}

