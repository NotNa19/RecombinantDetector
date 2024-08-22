#pragma once

#define StatisticalUtils_H


namespace StatisticalUtils {

	long double siegmundDiscreteApprox(const long& nUpSteps,
		const long& nDownSteps,
		const long& maxDescent);

	long double singleBreakPointPVal(const long& maxRWHeight,
		const long& nUpSteps,
		const long& nDownSteps);

	namespace {
		long double defaultSampleNumForPValCorrection = 1.0;
	}

	void setSampleNumForPValCorrection(const long double& sampleNumForPValCorrection);

	// Computes Dunn-Sidak correction for multiple comparisons. 
	long double dunnSidak(long double pVal,
		long double numTotalSamples = 0);

	// Computes Bonferroni correction for multiple comparisons. 
	long double bonferroni(long double pVal,
		long double numTotalSamples = 0);

}
