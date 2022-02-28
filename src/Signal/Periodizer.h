#include "ofMain.h"
/*!
	@brief Class to convert derivative aperiodic signals to periodic signals based source signal sampling rate.
*/
class Periodizer {
public:
	std::string _inputAperiodicSignal; //!< TypeTag of input derivative aperiodic signal
	std::string _inputPeriodicSignal;  //!< Typetag of input source signal
	std::string outputSignal;  //!< TypeTag of output periodized signal
	float _lastSampledValue;  //!< last aeriodic value received
	float _defaultValue; //!< default value to be used in the periodized output
						//!< by default set to NAN. The generated periodic output repeats the last sampled value
						//!< if set to a number, the generated output is filled with this value if a new sample
						//!< of the aperiodic signal is not received

	Periodizer();
	Periodizer(std::string inputApeSigId, std::string inputPeSigId, std::string outputSigId, float defaultOutputValue = NAN);
	/*!
		@brief updates the class variables or periodized output based on input
		@param identifier typetag of input data
		@param data the float data received from device
		@param periodizedData output periodized data
	*/
	size_t update(std::string identifier, std::vector<float> data, std::vector<float> &periodizedData);
};