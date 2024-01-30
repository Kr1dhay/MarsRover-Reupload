# Import required libraries
from inputs import get_gamepad
import math
import threading
import time
import socket

# Update this IP Address to the one on the Serial Monitor
UDP_IP = "192.168.0.201" 
UDP_PORT = 5555

# Double check IP Address and Port used 
print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

# Class to interface Xbox Controller
class XboxController(object):
    MAX_TRIG_VAL = math.pow(2, 8)
    MAX_JOY_VAL = math.pow(2, 15)

    def __init__(self):

        self.LeftJoystickY = 0
        self.LeftJoystickX = 0
        self.RightJoystickY = 0
        self.RightJoystickX = 0
        self.LeftTrigger = 0
        self.RightTrigger = 0

        self._monitor_thread = threading.Thread(target=self._monitor_controller, args=())
        self._monitor_thread.daemon = True
        self._monitor_thread.start()

    def read(self):  # return the buttons/triggers that you care about in this method
        xL = self.LeftJoystickX
        yL = self.LeftJoystickY
        xR = self.RightJoystickX
        yR = self.RightJoystickY
        left_trigger = self.LeftTrigger
        right_trigger = self.RightTrigger
        mid_val = 180
        if right_trigger :
            return 'f'
        else:
            if left_trigger:
                if xR < 0:
                    a = 1
                else:
                    a = 0
                if yR < 0:
                    b = 1
                else:
                    b = 0
                x = abs(xR)
                y = abs(yR)
                if 255 * x < 20 and 255 * y < 20:
                    return '1'
                elif b == 0 and y >= x:
                    return 'i'
                elif b == 1 and y > x:
                    return 'k'
                elif a == 1 and x >= y:
                    return 'j'
                elif a == 0 and x > y:
                    return 'l'
            else:
                if xL < 0:
                    a = 1
                else:
                    a = 0
                if yL < 0:
                    b = 1
                else:
                    b = 0
                x = abs(xL)
                y = abs(yL)
                if 255 * x < 20 and 255 * y < 20:
                    return '1'
                elif b == 0 and y >= x:
                    if 255 * y > mid_val:
                        return 'w'
                    else:
                        return 'i'
                elif b == 1 and y > x:
                    if 255 * y > mid_val:
                        return 's'
                    else:
                        return 'k'
                elif a == 1 and x >= y:
                    if 255 * x > mid_val:
                        return 'a'
                    else:
                        return 'j'
                elif a == 0 and x > y:
                    if 255 * x > mid_val: 
                        return 'd'
                    else:
                        return 'l'


    def _monitor_controller(self):
        while True:
            events = get_gamepad()
            for event in events:
                if event.code == 'ABS_Y':
                    self.LeftJoystickY = event.state / XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_X':
                    self.LeftJoystickX = event.state / XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_RY':
                    self.RightJoystickY = event.state / XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_RX':
                    self.RightJoystickX = event.state / XboxController.MAX_JOY_VAL  # normalize between -1 and 1
                elif event.code == 'ABS_Z':
                    self.LeftTrigger = event.state / XboxController.MAX_TRIG_VAL  # normalize between 0 and 1
                elif event.code == 'ABS_RZ':
                    self.RightTrigger = event.state / XboxController.MAX_TRIG_VAL # normalize between 0 and 1


if __name__ == '__main__':
    joy = XboxController()
    while True:
        print(joy.read())

        sock = socket.socket(socket.AF_INET,  
                             socket.SOCK_DGRAM) 

        packet = bytes(joy.read(), 'ascii')

        sock.sendto(packet, (UDP_IP, UDP_PORT))
        time.sleep(0.05) 

