####################################################################################################################################################
# Temporary Fix. Send to new script (listener). Then, output onto new script's GUI/Terminal. Run two scripts during execution. I am going to sleep # 
####################################################################################################################################################
import time
import socket

UDP_IP = "127.0.0.2"
UDP_PORT = 5006
rock = b"adamantine"  

sock = socket.socket(socket.AF_INET, # Internet
socket.SOCK_DGRAM) # UDP

sock.sendto(rock, (UDP_IP, UDP_PORT))