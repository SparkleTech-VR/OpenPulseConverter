#pragma once

class ExponentialFilter {
private:
	double alpha; // Weighting factor for the new data
	double filteredValue; // The filtered value
	double dataPoints[60]; // Array for holding the last 60 data points
	int currentIndex; // Index to keep track of the current position in the array

public:
	ExponentialFilter(double initial, double alpha) : filteredValue(initial), alpha(alpha), currentIndex(0) {
		for (int i = 0; i < 60; ++i) {
			dataPoints[i] = initial;
		}
	}

	double filter(double input) {


		dataPoints[currentIndex] = input; // Store the data point
		currentIndex = (currentIndex + 1) % 60; // Update the current index for the circular buffer

		double weightedSum = 0.0;
		double weight = 1.0;
		for (int i = 0; i < 60; ++i) {
			weightedSum += dataPoints[i] * weight;
			weight *= alpha; // Apply exponentially decreasing weight
		}

		filteredValue = weightedSum / 60;
		return filteredValue;
	}



	double* getDataPoints() {
		return dataPoints;
	}
};  // We need to make one rq and start using this filter below
