#include "AuxCxnController.h"

std::uint16_t AuxCxnController::begin()
{
	// start the UDP channel
	m_auxCxnUdp.Create();
	m_auxCxnUdp.SetNonBlocking(true);
	m_auxCxnUdp.SetReceiveBufferSize(pow(2, 13)); // 8KB
	m_auxCxnUdp.Bind(m_auxPort);

	// setup TCP channel
	// ToDo: setup for TCP channel

	return 0;
}

void AuxCxnController::readAuxCxn(AuxChannel channel)
{
	const int maxSize = 32768;
	static char udpMessage[maxSize];
	int msgSize = 0;
	static struct Remote {
		std::string ip;
		int port;
	}remote;
	if (channel == AuxChannel::CHANNEL_UDP)
	{
		// Read the UDP channel
		msgSize = m_auxCxnUdp.Receive(udpMessage, maxSize);
		m_auxCxnUdp.GetRemoteAddr(remote.ip, remote.port);
		if (msgSize)
		{
			// debug test code
			ofLogVerbose("AuxCxnController") << "UDP message received from remote (IP:port) = (" << ofToString(remote.ip) << ":" << ofToString(remote.port);
			ofLogVerbose("AuxCxnController") << "Raw message: " << udpMessage;
			// get packets from message
			std::vector<std::string> packets = parseMessage(udpMessage);
			// push packets to local queue
			vector<std::string>::iterator iter = packets.begin();
			for (iter; iter < packets.end(); iter++)
			{
				bool status = push(*iter);
				if (!status)
				{
					ofLogWarning("AuxCxnController") << "Lost data. Failed to push message into queue: " + *iter;
				}
			}
		}
	}
	else if (channel == AuxChannel::CHANNEL_TCP)
	{
		//ToDo: Impletement TCP channel read
	}
}


std::vector<std::string> AuxCxnController::parseMessage(std::string message)
{
	std::vector<std::string> packets;
	// ToDo: create a new function to parse message into packets. Currently copied from wifi host. ToDo: figure out the best place for this function, given it is shared between this controller and wifihost
	size_t startChar = 0;
	size_t endChar;
	std::string packet;
	// parse packets from received message
	do
	{
		endChar = message.find_first_of(EmotiBitPacket::PACKET_DELIMITER_CSV, startChar);
		if (endChar == string::npos)
		{
			ofLogWarning("AuxCxnController") << "**** MALFORMED MESSAGE **** : no packet delimiter found";
		}
		else
		{
			if (endChar == startChar)
			{
				ofLogWarning("AuxCxnController") << "**** EMPTY MESSAGE **** ";
			}
			else
			{
				packet = message.substr(startChar, endChar - startChar);	// extract packet
			}
			packets.push_back(packet);
		}
		startChar = endChar + 1;
	} while (endChar != string::npos && startChar < message.size());
	return packets;
}

bool AuxCxnController::push(std::string packet)
{
	if (m_bufferQ.size() < MAX_QUEUE_SIZE)
	{
		m_bufferQ.push(packet);
	}
	else
	{
		return false;
	}
	return true;
}

bool AuxCxnController::pop(std::string &packet)
{
	if (!m_bufferQ.empty())
	{
		packet = m_bufferQ.front();
		m_bufferQ.pop();
	}
	else
	{
		return false;
	}
	return true;
}

bool AuxCxnController::pushToAuxInstrQ()
{
	while (!m_bufferQ.empty())
	{
		if (m_mainQueue != nullptr)
		{
			ofLogVerbose("AuxCxnController") << "pushing to AuxInstrQ";
			m_mainQueue->push(m_bufferQ.front());
			m_bufferQ.pop();
		}
		else
		{
			ofLogError("AuxCxnController") << "nullptr exception! main application instruction queue is null";
			return false;
		}
	}
	return true;
}

bool AuxCxnController::attachMainQueue(AuxInstrQ *q)
{
	// ToDo: consider passing this as a constructor argument
	if (q != nullptr)
	{
		m_mainQueue = q;
		return true;
	}
	else
	{
		return false;
	}
}