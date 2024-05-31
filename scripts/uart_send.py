import time
import serial


ser = ''

def init_serial():
    global ser
    # configure the serial connections (the parameters differs on the device you are connecting to)
    ser = serial.Serial(
        port='/dev/ttyUSB0',
        baudrate=115200
    )

    ser.isOpen()



def serial_send(value):
    global ser
    ser.write(value)


