from argparse import ArgumentParser
from cqc.pythonLib import CQCConnection, qubit

# AWS host/port information for nodes
sock_info = {
    "Saravejo": ("", 8011),
    "Dublin": ("", 8014),
    "Delft": ("", 8017),
    "Paris": ("", 8020),
    "Padua": ("", 8023),
    "Geneva": ("", 8026)
}


def main():
    parser = ArgumentParser()
    parser.add_argument("host", type=str)
    args = parser.parse_args()
    for node, info in sock_info.items():
        sock_info[node] = (args.host, info[1])

    try:
        nodes = list(sock_info.keys())
        for i in range(len(nodes)):
            node1 = nodes[i]
            for j in range(i+1, len(nodes)):
                node2 = nodes[j]
                # Test creating EPR pairs between the nodes
                print("Testing EPR connection between {} and {}".format(node1, node2))
                for appID in range(10):
                    with CQCConnection(node1, socket_address=sock_info[node1], appID=appID) as conn1,\
                            CQCConnection(node2, socket_address=sock_info[node2], appID=appID) as conn2:

                        # Alternate between who sends and receives
                        conn_source, conn_sink = (conn1, conn2) if appID % 2 == 0 else (conn2, conn1)
                        print(conn_sink.name)
                        assert False
                        q1 = conn_source.createEPR(conn_sink.name, remote_appID=appID)
                        q2 = conn_sink.recvEPR()
                        m1 = q1.measure()
                        m2 = q2.measure()
                        if m1 != m2:
                            raise Exception("Measurement results for EPR test do not match!")

        print("Test 1: Successfully verified creating EPR pairs.")

        # Test sending qubits between nodes
        for i in range(len(nodes)):
            node1 = nodes[i]
            for j in range(i+1, len(nodes)):
                node2 = nodes[j]
                # Test sending qubits between the ndoes
                print("Testing sending qubits between {} and {}".format(node1, node2))
                for appID in range(10):
                    with CQCConnection(node1, socket_address=sock_info[node1], appID=appID) as conn1,\
                            CQCConnection(node2, socket_address=sock_info[node2], appID=appID) as conn2:

                        # Alternate between who sends and receives
                        conn_source, conn_sink = (conn1, conn2) if appID % 2 == 0 else (conn2, conn1)
                        q = qubit(conn_source)
                        conn_source.sendQubit(q, conn_sink.name, remote_appID=appID)
                        q = conn_sink.recvQubit()
                        q.measure()

        print("Test 2: Successfully verified sending qubits.")

    except Exception as err:
        print("Test failed, error: \n{}".format(err))


if __name__ == '__main__':
    main()
