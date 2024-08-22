#pragma once
#include <type_traits>
/**
 * The numeric values of this enum class were intentionally picked to support bitwise operations.
 */
enum class AllelicMask : unsigned char {
    Empty = 0b0000'0000,
    Gap = 0b0000'0001,
    Adenine = 0b0000'0010,
    Cytosine = 0b0000'0100,
    Guanine = 0b0000'1000,
    Thymine = 0b0001'0000,

    Mono = 0b0010'0000,
    Bi = 0b0100'0000,
    Tri = 0b1000'0000,
    Tetra = 0b0001'1110,

    TetraWithGap = 0b0001'1111
};

/**
 * Check if the left-hand-side (lhs) allelic mask has all the binary bits marked by the
 * right-hand-side (rhs) marker.
 * @param lhs The left-hand-side allelic marker.
 * @param rhs The right-hand-side allelic marker.
 * @return true if all 1-bits in the rhs also appear as 1-bits in the lhs;
 *         false otherwise.
 */
inline bool operator&(AllelicMask lhs, AllelicMask rhs) {
    return (static_cast<std::underlying_type<AllelicMask>::type>(lhs) &
        static_cast<std::underlying_type<AllelicMask>::type>(rhs))
        == static_cast<std::underlying_type<AllelicMask>::type>(rhs);
}

inline AllelicMask operator|(AllelicMask lhs, AllelicMask rhs) {
    return static_cast<AllelicMask> (
        static_cast<std::underlying_type<AllelicMask>::type>(lhs) |
        static_cast<std::underlying_type<AllelicMask>::type>(rhs));
}

inline AllelicMask& operator|=(AllelicMask& lhs, AllelicMask rhs) {
    return lhs = lhs | rhs;
}


