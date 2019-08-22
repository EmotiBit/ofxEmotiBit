#pragma once

#ifndef ARDUINO
#include <string>
#include <vector>
#include <iostream>
#endif

using namespace std;

class EmotiBitPacket {
public:

	// Platform independent fields

	class Header {
	public:
		uint32_t timestamp;
		uint16_t packetNumber;
		uint16_t dataLength;
#ifdef ARDUINO
		String typeTag;
#else
		std::string typeTag;
#endif
		uint8_t protocolVersion;
		uint8_t dataReliability;

		static const uint8_t length = 6;

		struct Structure {
			uint32_t timestamp;
			uint16_t packetNumber;
			uint16_t dataLength;
			char typeTag[2];
			uint8_t protocolVersion;
			uint8_t dataReliability;
		};
	};
	//Header header;


#ifdef ARDUINO
	// Arduino specific fields
#else
	// Non-arduino STL fields
	//vector<string> data;
#endif

	// Platform independent functions
	//EmotiBitPacket();
	//bool parsePacket(string packet);

#ifdef ARDUINO
	// Arduino specific functions
	//static String getTypeTag(const Header &h);
#else
	// Non-arduino STL functions
	//string getTypeTag();
	//static string getTypeTag(const Header &h);
	//Header getHeader();

#endif

	enum class PacketType {
		// EmotiBit Sensor Data
		EDA,	// Electrodermal Activity
		EDL,	// Electrodermal Level
		EDR,	// Electrodermal Response
		PPG_INFRARED,	// PPG Infrared
		PPG_RED,	// PPG Red
		PPG_GREEN,	// PPG Green
		TEMPERATURE_0,	// Temperature 0
		THERMISTOR,	// Thermistor
		HUMIDITY_0,	// Humidity 0
		ACCELEROMETER_X,	// Accelerometer X-axis
		ACCELEROMETER_Y,	// Accelerometer Y-axis
		ACCELEROMETER_Z,	// Accelerometer Z-axis
		GYROSCOPE_X,	// Gyroscope X-axis
		GYROSCOPE_Y,	// Gyroscope Y-axis
		GYROSCOPE_Z,	// Gyroscope Z-axis
		MAGNETOMETER_X,	// Magnetometer X-axis
		MAGNETOMETER_Y,	// Magnetometer Y-axis
		MAGNETOMETER_Z,	// Magnetometer Z-axis
		// Additional EmotiBit Info
		BATTERY_VOLTAGE,	// Battery Voltage
		BATTERY_PERCENT,	// Battery percent(%)
		DATA_CLIPPING,	// Data clipping, data = typeTags of data
		DATA_OVERFLOW,	// Data overflow, data = typeTags of data
		SD_CARD_PERCENT,	// SD card % full
		RESET,	// 
		// Computer -> EmotiBit Comms
		GPS_LATLNG,	// GPS Latitude & Longitude, data = latitude, longitude, accuracy
		GPS_SPEED,	// GPS Speed, data = speed, accuracy
		GPS_BEARING,	// GPS bearing, data = bearing, accuracy
		GPS_ALTITUDE,	// GPS altitude, data = altitude, verticalAccuracy
		TIMESTAMP_LOCAL,	// Local computer timestamp
		TIMESTAMP_UTC,	// UTC timestamp
		TIMESTAMP_CROSS_TIME, //Used for comparison between different clocks
		LSL_MARKER,
		RECORD_BEGIN,	// Record begin
		RECORD_END,	// Record end
		USER_NOTE,	// User note, data = char array
		MODE_HIBERNATE,	// Mode hibernate
		HELLO_EMOTIBIT,	// Hello Emotibit (used to establish communications)
		// General Comms
		ACK,
		REQUEST_DATA,	// Request data, data = list of requested data types
		PING,	// Ping
		PONG,	// Pong
		length	// number of packet types
	};

	//static const char* typeTags[]; // Iterable reference to all typeTags

	static class TypeTag {
	public:
		static const char* EDA;
		static const char* EDL;
		static const char* EDR;
		static const char* PPG_INFRARED;
		static const char* PPG_RED;
		static const char* PPG_GREEN;
		static const char* TEMPERATURE_0;
		static const char* THERMISTOR;
		static const char* HUMIDITY_0;
		static const char* ACCELEROMETER_X;
		static const char* ACCELEROMETER_Y;
		static const char* ACCELEROMETER_Z;
		static const char* GYROSCOPE_X;
		static const char* GYROSCOPE_Y;
		static const char* GYROSCOPE_Z;
		static const char* MAGNETOMETER_X;
		static const char* MAGNETOMETER_Y;
		static const char* MAGNETOMETER_Z;
		static const char* BATTERY_VOLTAGE;
		static const char* BATTERY_PERCENT;
		static const char* DATA_CLIPPING;
		static const char* DATA_OVERFLOW;
		static const char* SD_CARD_PERCENT;
		static const char* RESET;
		static const char* GPS_LATLNG;
		static const char* GPS_SPEED;
		static const char* GPS_BEARING;
		static const char* GPS_ALTITUDE;
		static const char* TIMESTAMP_LOCAL;
		static const char* TIMESTAMP_UTC;
		static const char* TIMESTAMP_CROSS_TIME;
		static const char* LSL_MARKER;
		static const char* RECORD_BEGIN;
		static const char* RECORD_END;
		static const char* USER_NOTE;
		static const char* MODE_HIBERNATE;
		static const char* HELLO_EMOTIBIT;
		static const char* ACK;
		static const char* REQUEST_DATA;
		static const char* PING;
		static const char* PONG;
        
//        static vector<string> APERIODIC;
	};
    
    static class TypeTagGroups{
    public:
        static const char* const APERIODIC[];
        static const char* const USER_MESSAGES[];
    };
	
    static const char PACKET_DELIMITER_CSV;
	static const string TIMESTAMP_STRING_FORMAT;

	EmotiBitPacket();

#ifndef ARDUINO
	static bool getHeader(vector<string> packet, Header &packetHeader); // Returns false if the packet is malformed
#endif

private:
	
};
