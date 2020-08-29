import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('localhost', 50002))
s.listen(1)
conn, addr = s.accept()

  #  data = conn.recv(1024)
  #  if not data:
  #      break
print("connected")
sleep(20)
conn.close()