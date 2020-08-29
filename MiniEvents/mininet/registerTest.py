import socket
from progmp import ProgMP
import os

os.system('sysctl -w net.mptcp.mptcp_enabled=1')
os.system('sysctl -w net.mptcp.mptcp_scheduler=rbs')
os.system('sysctl -w net.mptcp.mptcp_path_manager=fullmesh')
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("socket created")
s.connect(('localhost', 50002))
print("connected")
schedulerName = "illustratingMinRTT"
with open("illustratingMinRtt.progmp", "r") as src:
	schedProgStr = src.read()
try:
	ProgMP.loadScheduler(schedProgStr)
except:
	print "Scheduler loading error."
	
try:
	ProgMP.setScheduler(s, schedulerName)
except:
	print "Scheduler not found, maybe no MPTCP?"
	
ProgMP.setUser(s, 2)
ProgMP.setRegister(s, ProgMP.R1(), 5)
ProgMP.getRegister(s, ProgMP.R1())
s.send("Multipath is awesome!")