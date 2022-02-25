#include "Periodizer.h"

Periodizer::Periodizer()
{

}

Periodizer::Periodizer(std::string inputApeSigId, std::string inputPeSigId, std::string outputSigId, float defaultOutputValue)
{
	_inputAperiodicSignal = inputApeSigId;
	_inputPeriodicSignal = inputPeSigId;
	outputSignal = outputSigId;
	_defaultValue = defaultOutputValue;
	_lastSampledValue = 0;
}

size_t Periodizer::update(std::string identifier, std::vector<float> data, std::vector<float> &periodizedData)
{
	periodizedData.clear();
	// update class value if aperiodic data sample received
	if (identifier.compare(_inputAperiodicSignal) == 0)
	{
		// ToDo: have a better solution for handling situation when a aperiodic packet has multiple data points
		if (data.size() > 0)
		{
			_lastSampledValue = data.back();
		}
		return 0;
	}
	else if (identifier.compare(_inputPeriodicSignal) == 0)
	{
		// update output if base periodic signal is received
		if (isnan(_defaultValue)) // repeat previously received data
		{
			periodizedData.assign(data.size(), _lastSampledValue);

		}
		else   // push in default value 
		{
			// create vector with samples = #samples of base periodic signal
			periodizedData.assign(data.size(), _defaultValue);
			if (!isnan(_lastSampledValue))// new data point to plot
			{
				periodizedData.at(0) = _lastSampledValue;
				_lastSampledValue = NAN;
			}
		}
		return periodizedData.size();
	}
	else
	{
		return 0; // identifier is not related to the periodizer
	}
}