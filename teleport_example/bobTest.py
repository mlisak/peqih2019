from cqc.pythonLib import CQCConnection
import time

def main():
    t0 = time.time()
    # Initialize the connection
    with CQCConnection("Cenkovich") as Cenkovich:
        secret_key = ''
        for i in range(256):
            # Make an EPR pair with Alice
            qB = Cenkovich.recvEPR()
            # Receive info about corrections
            data = Cenkovich.recvClassical()
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
            """
            to_print = "App {}: Measurement outcome is: {}".format(Cenkovich.name, m)
            print("|" + "-" * (len(to_print) + 2) + "|")
            print("| " + to_print + " |")
            print("|" + "-" * (len(to_print) + 2) + "|")
            """
            #print("Measurement outcome is: " + str(m))

        print(hex(int(secret_key,2)))
    t1 = time.time()
    print('It took: ' + str(t1-t0) + ' seconds')
##################################################################################################
main()
