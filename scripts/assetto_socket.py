import socket
import unpack_struct
import uart_send
import struct
import time
from datetime import datetime
# infos: https://docs.google.com/document/d/1KfkZiIluXZ6mMhLWfDX1qAGbvhGRC3ZUzjVIt5FQpp4/pub


#############
# Globals

UDP_IP = "127.0.0.1"
UDP_PORT = 9996
last_time_sent = datetime.now()

    
#############
# Function

## Network

def socket_send(msg):
    sock.sendto(msg, (UDP_IP, UDP_PORT))

def socket_callback(callback: any):
    while(1):
        data_raw, addr = sock.recvfrom(1024)
        callback(data_raw, addr)

## Logic

def handshake():
    # first handshake
    socket_send(bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'))
    
    data_raw, _ = sock.recvfrom(408)

    data_dict = {
        "car_name": data_raw[0:50:2],
        "driver_name": data_raw[100:200:2],
        "identifier": int.from_bytes(data_raw[200:204], 'little'),
        "version": int.from_bytes(data_raw[204:208], 'little'),
        "track_name": data_raw[208:308:2],
        "track_config": data_raw[308:408:2],
    }

    # confirm we want updates
    socket_send(bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00'))

    return data_dict

def receive_n_send(data_raw, _):
    global last_time_sent
    live_data = unpack_struct.process(unpack_struct.live_structure_keys, unpack_struct.live_structure_fmt, data_raw)
    duration = datetime.now() - last_time_sent
    if(duration.microseconds > 900000):
        last_time_sent = datetime.now()
        print(live_data['speed_Kmh'], live_data['lapTime'], live_data['wheelAngularSpeed_0'])
        uart_send.serial_send(struct.pack("f", live_data['speed_Kmh']))
        uart_send.serial_send(struct.pack("I", live_data['lapTime']))
        uart_send.serial_send(struct.pack("f", live_data['wheelAngularSpeed_0']))
        uart_send.serial_send(struct.pack("f", live_data['gas']))
        uart_send.serial_send(struct.pack("?", live_data['isAbsEnabled']))
        uart_send.serial_send(struct.pack("?", live_data['isTcEnabled']))

    

#########
# Main

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

print("Listening...")

metadata = handshake()

uart_send.init_serial()   

socket_callback(receive_n_send)
 


