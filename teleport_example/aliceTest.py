from cqc.pythonLib import CQCConnection, qubit

def main():
    secret_key = ''
    # Initialize the connection
    with CQCConnection("Egemevo") as Egemevo:
        for i in range(256):
            # Make an EPR pair with Bob
            qA = Egemevo.createEPR("Cenkovich")
            # Create a random number
            q_random = qubit(Egemevo)
            q_random.H()
            r_number = q_random.measure()
            secret_key += str(r_number)
            #print('Random number: ' + str(r_number))
            # Create a qubit to teleport
            q = qubit(Egemevo)
            # Prepare the qubit to teleport in |+>
            if r_number==1:
                q.X()
            # Apply the local teleportation operations
            q.cnot(qA)
            q.H()
            # Measure the qubits
            a = q.measure()
            b = qA.measure()
            """
            to_print = "App {}: Measurement outcomes are: a={}, b={}".format(Egemevo.name, a, b)
            print("|" + "-" * (len(to_print) + 2) + "|")
            print("| " + to_print + " |")
            print("|" + "-" * (len(to_print) + 2) + "|")
            """
            # Send corrections to Bob
            Egemevo.sendClassical("Cenkovich", [a, b])
        print(hex(int(secret_key,2)))
main()

