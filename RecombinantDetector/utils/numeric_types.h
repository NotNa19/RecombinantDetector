#pragma once

#include <limits>
#include <cmath>

// We use this file for convenient work with numbers and some of their properties.
namespace Long {
    typedef long Type;
    const auto NOT_SET = std::numeric_limits<Type>::max();
    const auto MAX = std::numeric_limits<Type>::max() - 1;
}

namespace ULong {
    typedef size_t Type;
    const auto NOT_SET = std::numeric_limits<Type>::max();
    const auto MAX = std::numeric_limits<Type>::max() - 1;
}

namespace LLong {
    typedef long long Type;
    const Type NOT_SET = std::numeric_limits<Type>::max();
    const Type MAX = std::numeric_limits<Type>::max() - 1;
}

namespace Double {
    typedef double Type;
    const Type NOT_SET = std::numeric_limits<Type>::quiet_NaN();
    const Type MAX = std::numeric_limits<Type>::max();
}

namespace Float {
    typedef float Type;
    const Type NOT_SET = std::numeric_limits<Type>::quiet_NaN();
    const Type MAX = std::numeric_limits<Type>::max();
}


inline bool isSet(Long::Type numericVar) {
    return numericVar != Long::NOT_SET;
}


inline bool isSet(ULong::Type numericVar) {
    return numericVar != ULong::NOT_SET;
}


inline bool isSet(LLong::Type numericVar) {
    return numericVar != LLong::NOT_SET;
}


inline bool isSet(Double::Type numericVar) {
    return !std::isnan(numericVar);
}


inline bool isSet(Float::Type numericVar) {
    return !std::isnan(numericVar);
}
