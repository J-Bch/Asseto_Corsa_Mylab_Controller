import threading
import time
import uinput


class Gamepad:
    def __init__(
        self,
        sensibility = 1.0
    ):
        self.events = (
            uinput.BTN_A,
            uinput.BTN_B,
            uinput.BTN_X,
            uinput.BTN_Y,
            # uinput.BTN_0, uinput.BTN_1, uinput.BTN_2, uinput.BTN_3, uinput.BTN_4, uinput.BTN_5, uinput.BTN_6, uinput.BTN_7, uinput.BTN_8, uinput.BTN_9,
            uinput.ABS_X + (0, 255, 0, 0),
            uinput.ABS_Y + (0, 255, 0, 0),
            uinput.ABS_Z + (0, 255, 0, 0),
            # uinput.ABS_RX + (0, 255, 0, 0),
        )
        self.sensibility = sensibility
        self.rotation = 0
        self.brake = 0
        self.acceleration = 0
        self.btn_0 = 0
        self.btn_1 = 0
        self.btn_2 = 0
        self.btn_3 = 0
        

    def pt(self):
        with uinput.Device(self.events) as device:
                
            # device.emit(uinput.ABS_RX, 0)
            # device.emit(uinput.BTN_JOYSTICK, 0)
            # device.emit(uinput.BTN_0, 0)
            # device.emit(uinput.BTN_1, 0)
            # device.emit(uinput.BTN_2, 0)
            # device.emit(uinput.BTN_3, 0)
            # device.emit(uinput.BTN_4, 0)
            # device.emit(uinput.BTN_5, 0)
            # device.emit(uinput.BTN_6, 0)
            # device.emit(uinput.BTN_7, 0)
            # device.emit(uinput.BTN_8, 0)
            # device.emit(uinput.BTN_9, 0)
            test = True
            testo = True
            
            while True:
                
                # apply a factor on the delta between the rotation and the center (128)
                rotation_sensi_boost = (self.rotation - 128) * self.sensibility
                rotation_post = int(max(min(128 + rotation_sensi_boost, 255), 0))
                
                device.emit(uinput.ABS_X, rotation_post, syn=True)
                device.emit(uinput.ABS_Y, self.acceleration * 20)
                device.emit(uinput.ABS_Z, self.brake * 127)
                device.emit(uinput.BTN_A, self.btn_1)
                device.emit(uinput.BTN_B, self.btn_0)
                device.emit(uinput.BTN_X, self.btn_2)
                device.emit(uinput.BTN_Y, self.btn_3)

                test = not test
                testo = not testo
                time.sleep(0.1)

    def start(self):
        threading.Thread(target=self.pt).start()


# Device capabilites of logitech 920
# {
#     ('EV_SYN', 0): [
#             ('SYN_REPORT', 0),
#             ('SYN_CONFIG', 1),
#             ('SYN_DROPPED', 3),
#             ('?', 4),
#             ('?', 21)
#         ],
#     ('EV_KEY', 1): [
#             (['BTN_JOYSTICK', 'BTN_TRIGGER'], 288),
#             ('BTN_THUMB', 289),
#             ('BTN_THUMB2', 290),
#             ('BTN_TOP', 291),
#             ('BTN_TOP2', 292),
#             ('BTN_PINKIE', 293),
#             ('BTN_BASE', 294),
#             ('BTN_BASE2', 295),
#             ('BTN_BASE3', 296),
#             ('BTN_BASE4', 297),
#             ('BTN_BASE5', 298),
#             ('BTN_BASE6', 299),
#             ('?', 300),
#             ('?', 301),
#             ('?', 302),
#             ('BTN_DEAD', 303),
#             (['BTN_TRIGGER_HAPPY', 'BTN_TRIGGER_HAPPY1'], 704),
#             ('BTN_TRIGGER_HAPPY2', 705)
#     ],
#     ('EV_ABS', 3): [
#             (('ABS_X', 0), AbsInfo(value=33101, min=0, max=65535, fuzz=0, flat=0, resolution=0)),
#             (('ABS_Y', 1), AbsInfo(value=255, min=0, max=255, fuzz=0, flat=0, resolution=0)),
#             (('ABS_Z', 2), AbsInfo(value=255, min=0, max=255, fuzz=0, flat=0, resolution=0)), 
#             (('ABS_RZ', 5), AbsInfo(value=255, min=0, max=255, fuzz=0, flat=0, resolution=0)), 
#             (('ABS_HAT0X', 16), AbsInfo(value=0, min=-1, max=1, fuzz=0, flat=0, resolution=0)),
#             (('ABS_HAT0Y', 17), AbsInfo(value=0, min=-1, max=1, fuzz=0, flat=0, resolution=0))
#     ],
#     ('EV_MSC', 4): 
#         [
#             ('MSC_SCAN', 4)
#         ],
#     ('EV_FF', 21): [
#         (['FF_EFFECT_MIN', 'FF_RUMBLE'], 80),
#         ('FF_PERIODIC', 81),
#         ('FF_CONSTANT', 82),
#         ('FF_SPRING', 83),
#         ('FF_FRICTION', 84),
#         ('FF_DAMPER', 85),
#         ('FF_INERTIA', 86),
#         (['FF_EFFECT_MAX', 'FF_RAMP'], 87),
#         (['FF_SQUARE', 'FF_WAVEFORM_MIN'], 88),
#         ('FF_TRIANGLE', 89),
#         ('FF_SINE', 90),
#         ('FF_SAW_UP', 91),
#         ('FF_SAW_DOWN', 92),
#         (['FF_GAIN', 'FF_MAX_EFFECTS'], 96),
#         ('FF_AUTOCENTER', 97)
#     ]
# }
