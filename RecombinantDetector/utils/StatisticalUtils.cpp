#include "StatisticalUtils.h"

#include <cmath>

namespace StatisticalUtils {

	namespace {
		const long double PI = 3.14159265358979323846264338327950288419716939937510L;

		/**
		 * Calculate <i> 1 - exp(x) </i>.
		 * @param x A number
		 * @return <i>1 - exp(x)</i>.
		 * @Note
		 * If <i> x >= 0 </i>, the ordinary exponential function of C will be used.
		 * Otherwise, the result will be calculated by using the Taylor series: <br>
		 * <i> 1 - exp(x) = - x - (x^2)/(2!) - (x^3)/(3!) - (x^4)/(4!) - ... </i><br>
		 */
		long double oneSubtractExpX(long double x) {
			/* The number of terms in the Taylor series.
			 * The number is chosen based on the fact that floating point arithmetic
			 * is considered accurate up to 15 digits. */
			static const int MAX_TERMS_COUNT = 50;

			if (x >= 0.0L) {
				// If x is non-negative, the ordinary exp() function of C will be used.
				return 1.0L - expl(x);

			}
			else if (x >= -1.0L) {
				long double seriesTerms[MAX_TERMS_COUNT];
				long double xPower = 1.0L; // = x^0
				long double factorial = 1.0L; //  = 0!

				for (int i = 0; i < MAX_TERMS_COUNT; i++) {
					xPower *= x; //  = x^(i+1)
					factorial *= static_cast<long double> (i + 1); //  = (i+1)!

					seriesTerms[i] = xPower / factorial;
				}

				long double result = 0.0L;

				for (int i = MAX_TERMS_COUNT - 1; i >= 0; i--) {
					/* The order of adding the terms is important to reduce precision loss */
					result -= seriesTerms[i];
				}

				return result;

			}
			else {
				// x < 0 && |x| > 1
				long double absX = -x;
				long double expAbsX = expl(absX);
				return (expAbsX - 1.0L) / expAbsX;
			}
		}

		long double normPdf(long double x) {
			return (1.0L / sqrtl(2.0L * PI)) * expl(-0.5L * x * x);
		}

		long double normCdf(long double x) {
			static const long double B1 = 0.319381530L;
			static const long double B2 = -0.356563782L;
			static const long double B3 = 1.781477937L;
			static const long double B4 = -1.821255978L;
			static const long double B5 = 1.330274429L;
			static const long double P = 0.2316419L;
			static const long double C = 0.39894228L;

			if (x >= 0.0L) {
				long double t = 1.0L / (1.0L + P * x);
				return (
					1.0L - C * expl(-x * x / 2.0L) * t
					* (t * (t * (t * (t * B5 + B4) + B3) + B2) + B1)
					);

			}
			else {
				long double t = 1.0L / (1.0L - P * x);
				return (
					C * expl(-x * x / 2.0L) * t
					* (t * (t * (t * (t * B5 + B4) + B3) + B2) + B1)
					);
			}
		}

		long double nu(long double x) {
			return ((normCdf(x / 2.0L) - 0.5L) * 2.0L) / x
				/ (normPdf(x / 2.0L) + (x * normCdf(x / 2.0L)) / 2.0L);
		}


	}   // unnamed

	long double siegmundDiscreteApprox(const long& nUpSteps,
		const long& nDownSteps,
		const long& maxDescent) {
		auto downPlusUp =
			static_cast<long double> (nDownSteps) + static_cast<long double> (nUpSteps);
		auto downSubUp =
			static_cast<long double>(nDownSteps) - static_cast<long double>(nUpSteps);
		long double b = static_cast<long double>(maxDescent) - 0.5L;

		long double p1 = expl(-2.0L * b * (b - downSubUp) / downPlusUp);

		long double p2 = p1 * (2.0L * (2.0L * b - downSubUp) * (b - downSubUp) / downPlusUp + 1.0L);

		long double NU = nu(2.0L * (2.0L * b - downSubUp) / downPlusUp);

		long double p3 = NU * NU * p2;

		return oneSubtractExpX(-p3);
	}


	long double singleBreakPointPVal(const long& maxRWHeight,
		const long& nUpSteps,
		const long& nDownSteps) {
		long double closedPVal = 1.0;
		auto dMaxRWHeight = static_cast<long double>(maxRWHeight);
		auto dNUpSteps = static_cast<long double>(nUpSteps);
		auto dNDownSteps = static_cast<long double>(nDownSteps);

		for (long loopCount = 1; loopCount <= maxRWHeight; loopCount++) {
			closedPVal *= dNUpSteps / (dNDownSteps + dMaxRWHeight);
			dNUpSteps = dNUpSteps - 1.0;
			dNDownSteps = dNDownSteps - 1.0;
		}

		return closedPVal;
	}

	long double dunnSidak(long double pVal, long double numTotalSamples) {
		/* The minimum value of pVal that can be applied Dunn-Sidak correction */
		static const long double DS_THRESHOLD = 1e-15L;

		if (pVal >= 1.0L) return 1.0L;

		if (numTotalSamples <= 0) {
			numTotalSamples = defaultSampleNumForPValCorrection;
		}
		if (pVal < DS_THRESHOLD) {
			/* Simply return the Bonferroni correction because if the P-value is
			 * too small, then 1 - PValue rounds off to one and you cannot do a
			 * Dunn-Sidak correction. */
			return bonferroni(pVal, numTotalSamples);

		}
		else {
			return 1.0L - powl(1.0L - pVal, numTotalSamples);
		}
	}

	long double bonferroni(long double pVal, long double numTotalSamples) {
		if (numTotalSamples <= 0) {
			numTotalSamples = defaultSampleNumForPValCorrection;
		}
		return pVal * numTotalSamples;
	}

	void setSampleNumForPValCorrection(const long double& sampleNumForPValCorrection) {
		defaultSampleNumForPValCorrection = sampleNumForPValCorrection;
	}
}


