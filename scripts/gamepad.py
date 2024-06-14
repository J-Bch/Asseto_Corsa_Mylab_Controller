import threading
import time

import uinput


class Gamepad:
    def __init__(
        self, x_min: int = -100, x_max: int = 100
    ):
        self.events = (
            uinput.BTN_A,
            uinput.BTN_B,
            uinput.ABS_X + (x_min, x_max, 0, 0),
            uinput.ABS_Y + (x_min, x_max, 0, 0),
        )
        self.x_min = x_min
        self.x_max = x_max
        self.rotation = 0
        self.a = 0
        self.b = 0

    def pt(self):
        with uinput.Device(self.events) as device:
            while True:
                device.emit(uinput.ABS_Y, self.rotation, syn=False)
                device.emit(uinput.BTN_A, self.a)
                device.emit(uinput.BTN_B, self.b)
                time.sleep(0.1)

    def start(self):
        threading.Thread(target=self.pt).start()

