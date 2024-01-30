# Import required libraries
import socket

# Configure listener IP address and Port
UDP_IP = "192.168.0.197"
UDP_PORT = 55125

# Configure Socket to UDP
sock = socket.socket(socket.AF_INET,  
socket.SOCK_DGRAM) 

# Bind to IP Address and Port for listening
sock.bind((UDP_IP, UDP_PORT))

# To keep track of number of rocks
number = 1

# Infinite loop to enable continuous listening
while True:
    data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
    print(number) # prints current rock number
    print("received message: %s" % data) # prints rock type
    number += 1 
