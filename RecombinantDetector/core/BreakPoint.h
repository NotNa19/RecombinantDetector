#pragma once
#include <string>
#include <sstream>
#include <memory>

struct BreakPoint {
	size_t leftBound;
	size_t rightBound;
	long long height;

	explicit BreakPoint(const size_t& leftBound,
		const size_t& rightBound,
		const long long& height)
		: leftBound(leftBound), rightBound(rightBound), height(height) {};

	static std::shared_ptr<BreakPoint> create(const size_t& leftBound,
		const size_t& rightBound,
		const long long& height) {
		return std::make_shared<BreakPoint>(leftBound, rightBound, height);
	};

};

typedef std::shared_ptr<BreakPoint> BreakPointPtr;

struct BreakPointPair {
public:
	BreakPointPtr leftBreakPoint;
	BreakPointPtr rightBreakPoint;

	explicit BreakPointPair(BreakPointPtr leftBreakPoint,
		BreakPointPtr rightBreakPoint)
		: leftBreakPoint(std::move(leftBreakPoint)),
		rightBreakPoint(std::move(rightBreakPoint)) {};

	std::string toString() const {
		std::stringstream strStream;
		strStream << leftBreakPoint->leftBound << "-"
			<< leftBreakPoint->rightBound << " & "
			<< rightBreakPoint->leftBound << "-"
			<< rightBreakPoint->rightBound;
		return strStream.str();
	}
};

