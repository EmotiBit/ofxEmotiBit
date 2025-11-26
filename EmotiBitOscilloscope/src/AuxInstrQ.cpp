#include "AuxInstrQ.h"
#include "EmotiBitPacket.h"



bool AuxInstrQ::push(std::string packet)
{
	// ToDo: consider adding a max size for the queue
	q.push(packet);
	return true;
}

bool AuxInstrQ::pop(std::string &packet)
{
	if (!q.empty())
	{
		packet = q.front();
		q.pop();
	}
	return true;
}

size_t AuxInstrQ::getSize()
{
	return q.size();
}

bool AuxInstrQ::front(std::string &packet)
{
	if (q.size())
	{
		packet = q.front();
		return true;
	}
	return false;
}

void AuxInstrQ::updateLastPopTime(uint32_t popTime)
{
	lastPopTime = popTime;
}

uint32_t AuxInstrQ::getLastPopTime()
{
	return lastPopTime;
}

bool AuxInstrQ::clearStaleElement(uint32_t currentTime)
{
	if (currentTime - getLastPopTime() > MAX_TIME_WITHOUT_POP_MS)
	{
		if (q.size())
		{
			q.pop();
			return true;
		}
	}
	return false;
}