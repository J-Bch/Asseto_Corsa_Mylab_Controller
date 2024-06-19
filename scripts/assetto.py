import socket
import unpack_struct
import uart
import struct
import threading
import gamepad
from log import Log
from datetime import datetime
# infos: https://docs.google.com/document/d/1KfkZiIluXZ6mMhLWfDX1qAGbvhGRC3ZUzjVIt5FQpp4/pub

#--------------------------------------
# Globals
#--------------------------------------

# Constants
 
UDP_IP = "127.0.0.1"
UDP_PORT = 9996
SOCKET_TIMEOUT = 5
MESSAGE_SENT_WAIT_MICROSECONDS = 100000

# Variables

last_time_received = datetime.now()
last_time_sent = datetime.now()
message_counter = 0
pad = ()
log = ()

#--------------------------------------
# Functions
#--------------------------------------

def socket_send(msg):
    sock.sendto(msg, (UDP_IP, UDP_PORT))


 
# Handle communication with the game

def socket_callback(callback: any):
    
    global message_counter
    
    while(1):
        
        try:
        
            data_raw, addr = sock.recvfrom(1024)
            
            callback(data_raw, addr)
            
        except socket.timeout: 
            
            log.update_assetto_status("TIMEOUT")

            try :
                # this is  done so the dashboard can send a reset screen cmd to the driving wheel, INCLUDE ALL THE SAME NUMBER OF DATA OR WILL CAUSE A RESTART
                # uart.serial_send(struct.pack("?", True))
                # uart.serial_send(struct.pack("f", 0.0))
                # uart.serial_send(struct.pack("I", 0))
                # uart.serial_send(struct.pack("f", 0.0))
                # uart.serial_send(struct.pack("f", 0.0))
                # uart.serial_send(struct.pack("f", 0.0))
                # uart.serial_send(struct.pack("I", 0))
                # uart.serial_send(struct.pack("?", False))
                # uart.serial_send(struct.pack("?", False))
                # uart.serial_send(struct.pack("I", message_counter))
                
                message_counter += 1
                
            except:
                pass          

            handshake()
            

#--------------------------------------
# Logic
#--------------------------------------

def handshake():

    log.update_assetto_status('TRYING HANDSHAKE')

    global message_counter
    message_counter = 0
    
    data_raw = []

    recieved = False
    
    while(recieved == False):
        # first handshake
        socket_send(bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'))

        try:
            data_raw, _ = sock.recvfrom(408)
            if(data_raw):
                recieved = True
        except socket.timeout:
            log.update_assetto_status('TIMEOUT')

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
    
    log.update_assetto_status('CONNECTED')

    return data_dict



def receive_n_send(data_raw, _):
    
    global last_time_sent
    global message_counter

    # if not correct packet size
    if(len(data_raw) != 328):
        return
    
    # convert raw data from socket to a dictionnary for easy access
    live_data = unpack_struct.process(unpack_struct.live_structure_keys, unpack_struct.live_structure_fmt, data_raw)
    
    duration = datetime.now() - last_time_sent
    if(duration.microseconds > MESSAGE_SENT_WAIT_MICROSECONDS):
        last_time_sent = datetime.now()
        
        try:
            if(not uart.serial_is_open()):
                return
            
            uart.serial_send(struct.pack("?", False)) #reset screen bool
            uart.serial_send(struct.pack("f", live_data['speed_Kmh']))
            uart.serial_send(struct.pack("I", live_data['lapTime']))
            uart.serial_send(struct.pack("f", live_data['wheelAngularSpeed_0']))
            uart.serial_send(struct.pack("f", live_data['gas']))
            uart.serial_send(struct.pack("f", live_data['brake']))
            uart.serial_send(struct.pack("I", int(live_data["engineRPM"])))
            uart.serial_send(struct.pack("?", live_data['isAbsEnabled']))
            uart.serial_send(struct.pack("?", live_data['isTcEnabled']))
            uart.serial_send(struct.pack("I", message_counter))

        except:
            pass
                              
        message_counter += 1




def uart_thread():
    global log
    global last_time_received
    global pad
    
    while(1):
        ready = False
        
        while(not ready):
            
            try:
                uart.init_serial()
                ready = True
            except:
                log.update_dashboard_status('NOT CONNECTED')
                
        while(1):
            
            try:
                uart_recieve = uart.serial_get_line()
                
                # if wrong paquet size, discard
                if(len(uart_recieve)!= 6):
                    log.update_dashboard_status('TIMEOUT')
                    continue
                
                if(uart_recieve[0:5] == b'RESET'):
                    
                    log.update_dashboard_status('RESETING')
                    handshake()
                    uart.init_serial()
                    
                else:
                    
                    log.update_dashboard_status('CONNECTED')
                    pad.rotation = uart_recieve[0]
                    pad.brake = uart_recieve[1]
                    pad.acceleration = uart_recieve[2]
                    pad.btn_a = uart_recieve[3]
                    pad.btn_b = uart_recieve[4]
                    
            except:
                
                log.update_dashboard_status('NOT CONNECTED')
                ready = False
                
                break
            
    
#--------------------------------------
# Main
#--------------------------------------

# Display

log = Log()


# Virtual gamepad

pad = gamepad.Gamepad()
pad.start()


# Connexion with the dashboard

threading.Thread(target=uart_thread).start()


# Connexion with assetto corsa

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(SOCKET_TIMEOUT)
socket_callback(receive_n_send) 
handshake() 





