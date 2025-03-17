import socket

hostname = 'localhost'
port = 9088
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(10)
result = sock.connect_ex((hostname, port))
if result == 0:
    print(f"Connection to {hostname}:{port} succeeded")
else:
    print(f"Connection to {hostname}:{port} failed")


