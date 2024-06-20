import serial


ser = ''

def init_serial():
    global ser
    
    if ser != '':
        ser.close()
    
    # configure the serial connections (the parameters differs on the device you are connecting to)
    ser = serial.Serial(
        port='/dev/ttyUSB0',
        baudrate=115200,
        timeout=1
    )
    ser.isOpen()


def serial_is_open():
    global ser
    
    if ser == '':
        return False
    
    return ser.isOpen()

def serial_send(value):
    global ser
    ser.write(value)


def serial_get_nonblock():
    global ser
    value = ser.read_all()
    return value

def serial_get_line():
    global ser
    value = ser.read_until(b'\xff')
    return value


def serial_get_data():
    global ser
    value = ser.read(5)
    return value


