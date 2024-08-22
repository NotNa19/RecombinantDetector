#pragma once
enum class Nucleotide : char {
    Adenine = 'A',
    Thymine = 'T',
    Guanine = 'G',
    Cytosine = 'C',
    Gap = '-'
};

inline bool operator==(const Nucleotide& left, const char& right) {
    return static_cast<char>(left) == right;
}

inline bool operator==(const char& left, const Nucleotide& right) {
    return left == right;
}

inline bool operator!=(const Nucleotide& left, const char& right) {
    return static_cast<char>(left) != right;
}

inline bool operator!=(const char& left, const Nucleotide& rhs) {
    return left != rhs;
}

