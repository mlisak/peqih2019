from cqc.pythonLib import CQCConnection, qubit
import time
def main():
    # Egemevo --> SEND
    # Cenkovich --> RECV
    # device_name = 'Egemevo'
    comm         = 'SEND'
    if comm=='RECV':
        recv_conn()
    elif comm=='SEND':
        create_conn()
            
def create_conn(device_name='Egemevo',target_name='Cenkovich'):
    secret_key = ''
    with CQCConnection(device_name) as Alice:
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

def recv_conn(device_name="Cenkovich"):
    t0 = time.time()
    # Initialize the connection
    with CQCConnection(device_name) as Bob:
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
    print('It took: ' + str(t1-t0) + ' seconds')

main()
