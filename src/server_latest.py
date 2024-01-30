# Import required libraries
import customtkinter
import os
import threading
import time
import socket
import joystick

customtkinter.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"

# set variables for rover port (to modify when using EEERover network) - Transmit
UDP_IP = "192.168.0.201"  # Update this IP Address to the one on the Serial Monitor
UDP_PORT = 5555
FORWARD = b"w"
BACKWARD = b"s"
LEFT = b"a"
RIGHT = b"d"
DETECT = b"f"
STOP = b"1"

# Data variable from rover
# rockReturned = "NO ROCK DETECTED"

keyboard = "TRUE"

PATH = os.path.dirname(os.path.realpath(__file__))

# double check variables in terminal
print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)

# configuring tx socket to UDP
sock = socket.socket(socket.AF_INET,  # Internet
                     socket.SOCK_DGRAM)  # UDP


# functions for packet sending
def forward():
    if keyboard == "TRUE":
        sock.sendto(FORWARD, (UDP_IP, UDP_PORT))
        print('w')


def backward():
    if keyboard == "TRUE":
        sock.sendto(BACKWARD, (UDP_IP, UDP_PORT))
        print('s')


def left():
    if keyboard == "TRUE":
        sock.sendto(LEFT, (UDP_IP, UDP_PORT))
        print('a')


def right():
    if keyboard == "TRUE":
        sock.sendto(RIGHT, (UDP_IP, UDP_PORT))
        print('d')

def stop():
    if keyboard == "TRUE":
        sock.sendto(STOP, (UDP_IP, UDP_PORT))

# def open_popup():
#     win = tkinter.Toplevel()
#     win.wm_title("Rock Found:")
#
#     win.geometry("180x50")
#     win.configure(bg="black")
#
#     l = tkinter.Label(win, text=rockReturned, font="Roboto", padx=60)
#     l.grid(row=0, column=0)


def detect():
    if keyboard == "TRUE":
        sock.sendto(DETECT, (UDP_IP, UDP_PORT))
        # #Asynchronously stop sending data, and start listening from client.
        # import client
        # from client import data
        # global rockReturned
        # rockReturned = data
        # open_popup()
        print("detect")


def run():
    while True:
        print('thread running')
        time.sleep(1)
        global stop_threads
        if stop_threads:
            break

def do_joystick():
    joy = joystick.XboxController()
    while True:
        print(joy.read())
        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP

        packet = bytes(joy.read(), 'ascii')

        sock.sendto(packet, (UDP_IP, UDP_PORT))
        time.sleep(0.05)
        global stop_threads
        if stop_threads:
            break


class App(customtkinter.CTk):
    WIDTH = 700
    HEIGHT = 520

    def __init__(self):
        super().__init__()

        self.title("Rover Control Interface")
        self.geometry(f"{App.WIDTH}x{App.HEIGHT}")
        self.protocol("WM_DELETE_WINDOW", self.on_closing)  # call .on_closing() when app gets closed

        # ============ create two frames ============

        # configure grid layout (2x1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)

        self.frame_left = customtkinter.CTkFrame(master=self,
                                                 width=180,
                                                 corner_radius=0)
        self.frame_left.grid(row=0, column=0, sticky="nswe")

        self.frame_right = customtkinter.CTkFrame(master=self)
        self.frame_right.grid(row=0, column=1, sticky="nswe", padx=20, pady=20)

        # ============ frame_left ============

        # configure grid layout (1x11)
        self.frame_left.grid_rowconfigure(0, minsize=10)  # empty row with minsize as spacing
        self.frame_left.grid_rowconfigure(5, weight=1)  # empty row as spacing
        self.frame_left.grid_rowconfigure(8, minsize=20)  # empty row with minsize as spacing
        self.frame_left.grid_rowconfigure(11, minsize=10)  # empty row with minsize as spacing

        self.label_1 = customtkinter.CTkLabel(master=self.frame_left,
                                              text="AJFE2 ROVER",
                                              text_font=("Roboto Medium", -16))  # font name and size in px
        self.label_1.grid(row=1, column=0, pady=10, padx=10)

        self.switch_1 = customtkinter.CTkSwitch(master=self.frame_left,
                                                text="Controller/Keyboard",
                                                command=self.change_input)
        self.switch_1.grid(row=9, column=0, pady=10, padx=20)

        self.switch_2 = customtkinter.CTkSwitch(master=self.frame_left,
                                                text="Dark Mode",
                                                command=self.change_mode)
        self.switch_2.grid(row=10, column=0, pady=10, padx=20)

        # ============ frame_right ============

        # configure grid layout (3x7)
        self.frame_right.rowconfigure((0, 1, 2, 3), weight=1)
        self.frame_right.rowconfigure(7, weight=10)
        self.frame_right.columnconfigure((0, 1), weight=1)
        self.frame_right.columnconfigure(2, weight=0)

        self.frame_info = customtkinter.CTkFrame(master=self.frame_right)
        self.frame_info.grid(row=0, column=0, columnspan=4, rowspan=4, pady=20, padx=20, sticky="nsew")
        # ============ frame_info ============

        # configure grid layout (1x1)
        self.frame_info.rowconfigure(0, weight=1)
        self.frame_info.columnconfigure(0, weight=1)

        # ============ frame_right ============

        self.slider_button_1 = customtkinter.CTkButton(master=self.frame_right,
                                                       height=25,
                                                       text="W",
                                                       command=forward)
        self.slider_button_1.grid(row=5, column=2, columnspan=1, pady=10, padx=20, sticky="we")

        self.slider_button_2 = customtkinter.CTkButton(master=self.frame_right,
                                                       height=25,
                                                       text="A",
                                                       command=left)
        self.slider_button_2.grid(row=6, column=1, columnspan=1, pady=10, padx=20, sticky="we")

        self.slider_button_3 = customtkinter.CTkButton(master=self.frame_right,
                                                       height=25,
                                                       text="S",
                                                       command=backward)
        self.slider_button_3.grid(row=6, column=2, columnspan=1, pady=10, padx=20, sticky="we")

        self.slider_button_4 = customtkinter.CTkButton(master=self.frame_right,
                                                       height=25,
                                                       text="D",
                                                       command=right)
        self.slider_button_4.grid(row=6, column=3, columnspan=1, pady=10, padx=20, sticky="we")

        self.checkbox_button_1 = customtkinter.CTkButton(master=self.frame_right,
                                                         height=25,
                                                         text="SENSE",
                                                         border_width=3,  # <- custom border_width
                                                         fg_color=None,  # <- no fg_color
                                                         command=detect,
                                                         )
        self.checkbox_button_1.grid(row=8, column=1, columnspan=3, pady=10, padx=20, sticky="we")

        # set default values
        self.switch_2.select()
        self.switch_1.select()

    # def button_event(self):
    #     print("Button pressed")

    def change_input(self):
        global stop_threads
        stop_threads = False
        t1 = threading.Thread(target=do_joystick)

        if self.switch_1.get() == 1:
            global keyboard
            keyboard = "TRUE"
            print('true')
            stop_threads = True
            print('thread killed')
        else:
            keyboard = "FALSE"
            print('false')
            t1.start()

    def change_mode(self):
        if self.switch_2.get() == 1:
            customtkinter.set_appearance_mode("dark")
        else:
            customtkinter.set_appearance_mode("light")

    def on_closing(self):
        self.destroy()


if __name__ == "__main__":
    app = App()
    # Tkinter event loop
    # define flags
    w_pressed = True
    s_pressed = True
    a_pressed = True
    d_pressed = True
    f_pressed = True
    if keyboard == "TRUE":
        def keyPressed(event):
            global w_pressed
            global s_pressed
            global a_pressed
            global d_pressed
            global f_pressed
            if event.char == "w":
                if w_pressed:
                    forward()
                    w_pressed = False
            elif event.char == "s":
                if s_pressed:
                    backward()
                    s_pressed = False
            elif event.char == "a":
                if a_pressed:
                    left()
                    a_pressed = False
            elif event.char == "d":
                if d_pressed:
                    right()
                    d_pressed = False
            elif event.char == "f":
                if f_pressed:
                    detect()
                    f_pressed = False


        def keyReleased(event):
            global w_pressed
            global s_pressed
            global a_pressed
            global d_pressed
            global f_pressed
            if event.char == "w":
                stop()
                w_pressed = True
            elif event.char == "s":
                stop()
                s_pressed = True
            elif event.char == "a":
                stop()
                a_pressed = True
            elif event.char == "d":
                stop()
                d_pressed = True
            elif event.char == "f":
                stop()
                f_pressed = True

        app.bind("<KeyPress>", keyPressed)
        app.bind("<KeyRelease>", keyReleased)

    app.mainloop()
