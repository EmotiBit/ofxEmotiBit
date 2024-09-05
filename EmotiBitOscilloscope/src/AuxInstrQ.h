#pragma once
#include <queue>
class AuxInstrQ {
	std::queue<std::string> q;
	uint32_t lastPopTime = 0;
	const uint16_t MAX_TIME_WITHOUT_POP_MS = 1000;  ///< Max time an element can stay at the top of the queue in mS

public:
	
	/**
	 * \brief Pushes elements into the class queue.
	 * 
	 * \param packet Packet to push into the queue.
	 * \return true is successful, else false
	 */
	bool push(std::string packet);

	/**
	 * \brief Pops element out of the queue.
	 * 
	 * \param packet Element popped out of the queue is stored in packet
	 * \return true is successful, else false
	 */
	bool pop(std::string &packet);

	/**
	 * \brief Peek at the front element of the queue.
	 * 
	 * \param packet Stores the front element of the queue
	 * \return true if queue is not empty
	 */
	bool front(std::string &packet);

	/**
	 * \brief Get size of the queue.
	 * 
	 * \return size of the queue
	 */
	size_t getSize();

	/**
	 * \brief Function to record the time when element is popped from the queue. Used internally to remove stale queue elements.
	 * 
	 */
	void updateLastPopTime();

	/**
	 * \brief Returns last time-since-application-start when queue elelment was popped.
	 * 
	 * \return time of last pop in ms
	 */
	uint32_t getLastPopTime();

	/**
	 * \brief Pops front element if it has not been popped for more than set timeout.
	 * 
	 * \return true if stale element is popped 
	 */
	bool clearStaleElement();
};