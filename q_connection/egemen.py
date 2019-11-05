from argparse import ArgumentParser
from cqc.pythonLib import CQCConnection, qubit

# AWS host/port information for nodes
sock_info = {
    "Cenkovich": ("", 8061),
    "Egemevo": ("", 8071),
}


def main():
    with CQCConnection("Egemen", socket_address=("localhost", 8000)) as conn1:
        while True:
            try:
                conn1.createEPR()
                q = source.recvQubit()
                q.measure()
            except:
                print("Waiting connection.", end="\r")
 
        print("Test 2: Successfully verified sending qubits.")


if __name__ == '__main__':
    main()
