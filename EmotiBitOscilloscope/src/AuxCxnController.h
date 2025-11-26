#pragma once
/* Controller for auxillary network channel*/
#include <queue>
#include <ofxUDPManager.h>
//#include <ofxTCPClient.h> // ToDo
#include <ofLog.h>
#include <ofUtils.h>
#include "EmotiBitPacket.h"
#include "AuxInstrQ.h"
#include "EmotiBitComms.h"

class AuxCxnController {
	const uint32_t UDP_CXN_BUFFER_SIZE = 8192; // 8KB
public:
	enum AuxChannel {
		CHANNEL_UDP = 0,
		CHANNEL_TCP
	};
	int auxPort = EmotiBitComms::WIFI_ADVERTISING_PORT - 1;  ///< Port where auxillary messages can be sent to the oscilloscope
	const int MAX_QUEUE_SIZE = 1000; // random number chosen for now
	
	ofxUDPManager auxCxnUdp;
	//ofxTCPClient auxCxnTcp; ToDo
	AuxInstrQ *appQ;  ///< pointer to main application queue
	
	/*!
	* \brief Setup the AUX Cxn
	* \return 0 is success, else failure code
	*/
	std::uint16_t begin();

	/*!
	* \brief Read the aux port for incoming messages
	* \param channel Either UDP or TCP
	*/
	void readAuxCxn(AuxChannel channel = AuxChannel::CHANNEL_UDP);

	/*!
	* \brief Parse received UDP/TCP message into packets. Set packet delimiter is used to parse message into packets
	* \return Instruction packet(s)
	*/
	std::vector<std::string> parseMessage(std::string message);
	   

	/*!
	* \brief push to local queue. All UDP/TCP messages are temporarily stored in this queue before being pushed to the main queue in the application.
	* \return true if pushed 
	*/
	bool push(std::string packet);



	/*!
	* \brief pop queue element.
	* \return True if queue is not empty.
	*/
	bool pop(std::string &packet);

	/*!
	* \brief Pushes all messages from local controller queue to main application queue
	* \return true is successful
	*/
	bool pushToAppQ();

	/**
	 * \brief Function called in ofApp during setup to pass main application queue pointer to this controller.
	 * 
	 * \param q pointer to main application queue
	 * \return true if pointer is not null
	 */
	bool attachAppQ(AuxInstrQ *q);

private:

	std::queue<std::string> bufferQ;  ///< Local queue to buffer UDP/TCP messages before pushing into the main application queue
	
};