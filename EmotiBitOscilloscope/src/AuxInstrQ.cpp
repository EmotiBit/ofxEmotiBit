#include "AuxInstrQ.h"
#include "EmotiBitPacket.h"



bool AuxInstrQ::push(std::string packet)
{
	// ToDo: consider adding a max size for the queue
	m_queue.push(packet);
	return true;
}

bool AuxInstrQ::pop(std::string &packet)
{
	if (!m_queue.empty())
	{
		packet = m_queue.front();
		m_queue.pop();
	}
	return true;
}

size_t AuxInstrQ::getSize()
{
	return m_queue.size();
}

bool AuxInstrQ::front(std::string &packet)
{
	if (m_queue.size())
	{
		packet = m_queue.front();
		return true;
	}
	return false;
}

void AuxInstrQ::updateLastPopTime()
{
	m_lastPopTime = ofGetElapsedTimeMillis();
}

uint32_t AuxInstrQ::getLastPopTime()
{
	return m_lastPopTime;
}

bool AuxInstrQ::clearStaleElement()
{
	if (ofGetElapsedTimeMillis() - getLastPopTime() > MAX_TIME_WITHOUT_POP_MS)
	{
		if (m_queue.size())
		{
			m_queue.pop();
			return true;
		}
	}
	return false;
}