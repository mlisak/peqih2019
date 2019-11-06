import socket
import sys


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 10000)
print('connecting to {} port {}'.format(*server_address))
sock.connect(server_address)

try:
    message = str.encode(str(sys.argv[1])) 
    print('sending {!r}'.format(message))
    sock.sendall(message)

    # Look for the response
    amount_received = 0
    amount_expected = len(message)

    while amount_received <= amount_expected:
        data = sock.recv(66)
        amount_received += len(data)
        if len(data)==66:
            print(data.decode("utf-8"))

finally:
    print('closing socket')
    sock.close()