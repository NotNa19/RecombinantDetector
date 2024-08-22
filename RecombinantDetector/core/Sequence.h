#pragma once
#include "Nucleotide.h"

#include <vector>
#include <string>
#include <memory>

class Alignment;
class Triplet;

class Sequence {
public:
	enum class RecombinantType {
		NotRec,
		Short,
		Long
	};

	// For fast access to fields
	friend class Alignment;
	friend class Triplet;

	static std::shared_ptr<Sequence> create(const std::string& name, const std::string& dna);
	explicit Sequence(std::string name, const std::string& dnaString);

	Sequence(const Sequence& rhs) = delete;
	Sequence(Sequence&&) = delete;
	Sequence& operator=(const Sequence&) = delete;
	Sequence& operator=(Sequence&&) = delete;

	virtual ~Sequence();

	std::string toString() const;

	size_t activeLength() const;
	size_t fullLength() const;
	size_t countGaps() const;

	const std::string& name() const;

	size_t getOriginalPositionOfActiveNuc(const size_t idx) const;

	const Nucleotide& getActiveNuc(const size_t idx) const;

	size_t computeDistance(const Sequence& other, bool const& ignoreGaps = true) const;

	/* Checks if this sequence is similar to other.
	 * Gap elements from this considered to be equal with every element from the other
	 */
	bool isSimilarWithGaps(const Sequence& other) const;

	RecombinantType getRecombinantType() const;

	void setRecombinantType(RecombinantType newRecombinantType);

	const std::vector<Nucleotide>& nucleotides() const { return m_nucleotides; };
	time_t data() const { return m_data; };

	bool isOlderThan(const Sequence& other) const;
private:

	/* 1) Capitalise all characters
	 * 2) replace Uuracil with Thymine
	 * 3) Replace all non-nucleotide character with gap
	 */
	static std::string preprocessInput(const std::string& dna);
private:
	const std::string m_name;
	const time_t m_data;

	std::vector<Nucleotide> m_nucleotides;
	std::shared_ptr<std::vector<size_t>> m_activePositions;
	RecombinantType m_recombinantType;
};

typedef std::shared_ptr<Sequence> SequencePtr;