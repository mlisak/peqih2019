from cqc.pythonLib import CQCConnection, qubit
import time
import socket
import sys
import json
def main():
    device_name = str(sys.argv[1])
    if device_name == 'cenkmac':
        target_name = 'bulut'
    elif device_name == "bulut":
        target_name = 'cenkmac'
    else:
        assert False, "Unspecified device name %s" % device_name
    
    conn,comm = listen()
    if comm==b'RECV':
        recv_conn(conn,device_name)
    elif comm==b'SEND':
        create_conn(conn,device_name,target_name)

def create_conn(conn,device_name,target_name):
    t0 = time.time()
    print('Sending RECV from %s to %s' %(device_name,target_name))
    #send_rec(device_name,target_name)
    print('Starting creating and sending QK from: %s to: %s' %(device_name,target_name) )
    secret_key = ''
    with CQCConnection(device_name) as Alice:
        print('start comm')
        for i in range(256):
            # Make an EPR pair with Bob
            qA = Alice.createEPR(target_name)
            # Create a random number
            q_random = qubit(Alice)
            q_random.H()
            r_number = q_random.measure()
            secret_key += str(r_number)
            #print('Random number: ' + str(r_number))
            # Create a qubit to teleport
            q = qubit(Alice)
            # Prepare the qubit to teleport in |+>
            if r_number==1:
                q.X()
            # Apply the local teleportation operations
            q.cnot(qA)
            q.H()
            # Measure the qubits
            a = q.measure()
            b = qA.measure()
            Alice.sendClassical(target_name, [a, b])
        print(hex(int(secret_key,2)))
    t1 = time.time()
    secs = t1-t0
    print('It took: %d mins %d seconds' % (secs // 60, secs % 60))
    send(conn,hex(int(secret_key,2))) # Sending the result back
def recv_conn(conn,device_name):
    conn.close()
    print('Starting receiving QK')
    t0 = time.time()
    # Initialize the connection
    with CQCConnection(device_name) as Bob:
        print('start comm')
        secret_key = ''
        for i in range(256):
            # Make an EPR pair with Alice
            qB = Bob.recvEPR()
            # Receive info about corrections
            data = Bob.recvClassical()
            message = list(data)
            a = message[0]
            b = message[1]
            # Apply corrections
            if b == 1:
                qB.X()
            if a == 1:
                qB.Z()
            # Measure qubit
            m = qB.measure()
            secret_key += str(m)
        print(hex(int(secret_key,2)))
    t1 = time.time()
    secs = t1-t0
    print('It took: %d mins %d seconds' % (secs // 60, secs % 60))
    send(conn,hex(int(secret_key,2)))
def listen():
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Bind the socket to the port
    server_address = ('localhost', 10001)
    print('starting up on {} port {}'.format(*server_address))
    sock.bind(server_address)
    # Listen for incoming connections
    sock.listen(1)
    while True:
        # Wait for a connection
        print('waiting for a connection')
        connection, client_address = sock.accept()
        try:
            print('connection from', client_address)
            # Receive the data in small chunks and retransmit it
            while True:
                data = connection.recv(4)
                #print('received {!r}'.format(data))
                if data:
                    return connection,data
        except:
            pass
def send(connection,message):
    connection.sendall(str.encode(str(message)))
    connection.close()
def send_rec(device_name,target_name):
    with open('network.json') as f:
        foo = json.load(f)
        target_ip,target_port = foo["default"]["nodes"][target_name]["app_socket"] 
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Connect the socket to the port where the server is listening
    server_address = (target_ip, 10000)
    print('connecting to {} port {}'.format(*server_address))
    sock.connect(server_address)
    try:
        message = str.encode('RECV') 
        print('sending {!r}'.format(message))
        sock.sendall(message)
    finally:
        print('closing socket')
        sock.close()

main()
