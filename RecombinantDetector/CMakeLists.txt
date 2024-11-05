cmake_minimum_required (VERSION 3.8)

project ("RecDetector")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCES
    app/App.cpp
    app/FastaReader.cpp
    app/PhyloLocator.cpp
    app/PTable.cpp
    app/PTableFile.cpp
    app/TextFile.cpp
    app/UserSettings.cpp
    app/modes/PTableGenerator.cpp
    app/modes/RecombinantDetector.cpp
    app/modes/Run.cpp
    core/Triplet.cpp
    core/TripletPool.cpp
    core/PhyloNode.cpp
    core/PhyloTree.cpp
    core/Sequence.cpp
    core/SequencePool.cpp
    core/Alignment.cpp
    core/AlignmentDescriptor.cpp
    utils/Utils.cpp
    utils/StatisticalUtils.cpp
    utils/StringUtils.cpp
)

include_directories(
    .
    ./app
    ./app/modes
    ./core
    ./utils
    ./include
)

add_executable (RecDetector ${SOURCES})