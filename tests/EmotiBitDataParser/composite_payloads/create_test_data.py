import sys
import os
import EmotiBitPacket
import argparse

def create_EM_packet(packet_number):
    # packet example: 24187,1518,4,EM,1,100,RS,RB,2025-03-20_12-09-40-822726.csv,PS,MN
    payload = ["RS","RB","2025-03-20_12-09-40-822726.csv","PS","MN"]
    packet = EmotiBitPacket.createPacket("EM",packet_number,payload,1,100)
    return packet

def create_DC_packet(packet_number):
    payload = ["AZ","1","GX","1"]
    packet = EmotiBitPacket.createPacket("DC",packet_number,payload,1,100)
    return packet

def create_AK_packet(packet_number):
    payload = ["5469","RD"]
    packet = EmotiBitPacket.createPacket("AK",packet_number,payload,1,100)
    return packet

def create_RD_packet(packet_number):
    payload = ["TL","TU"]
    packet = EmotiBitPacket.createPacket("RD",packet_number,payload,1,100)
    return packet


def main():
    packet_number=1
    dir_path = "./test_data"
    file_name = "test_data.csv"
    full_file_path = os.path.join(dir_path, file_name)
    try:
        with open(full_file_path, 'w') as file:
            file.write(create_EM_packet(packet_number))
            packet_number += 1
            file.write(create_DC_packet(packet_number))
            packet_number += 1
            file.write(create_AK_packet(packet_number))
            packet_number += 1
            file.write(create_RD_packet(packet_number))
            packet_number += 1
    except IOError as e:
        print(f"Error writing to file: {e}")
if __name__ == "__main__":
    main()
