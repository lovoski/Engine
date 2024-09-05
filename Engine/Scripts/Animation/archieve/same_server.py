import socket
import os
import signal
import sys
import time
from fairmotion.data import bvh
from skeleton_encoder_test import neural_retargeting

global_retarget_motion_counter = 0
def handle_same_retargeting(target_skeleton_file, source_motion_file):
    print(f"skeleton file: {target_skeleton_file}, motion file: {source_motion_file}")
    
    retarget_motion = neural_retargeting(target_skeleton_file, source_motion_file)
    global global_retarget_motion_counter
    retarget_motion_relpath = f"result_{global_retarget_motion_counter}.bvh"
    bvh.save(retarget_motion, retarget_motion_relpath)
    global_retarget_motion_counter += 1
    
    retarget_motion_abspath = os.path.abspath(retarget_motion_relpath)
    print(f"neural retargeting finished, save results to {retarget_motion_abspath}")
    
    return retarget_motion_abspath + ";"


def start_server():
    host = "127.0.0.1"
    port = 9999

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen()
        server_socket.settimeout(1.0)  # Set a timeout of 1 second
        print(f"Server started at {host}:{port}")

        try:
            while True:
                try:
                    client_socket, client_address = server_socket.accept()
                    print(f"Connection from {client_address}")

                    with client_socket:
                        data = client_socket.recv(1024).decode("utf-8")
                        if data:
                            print(f"Received: {data}")

                            # handle same retargeting
                            segs = data.split(";")
                            retarget_file = handle_same_retargeting(segs[0], segs[1])

                            client_socket.sendall(retarget_file.encode())
                except socket.timeout:
                    continue  # Timeout expired, continue the loop to check for KeyboardInterrupt
        except KeyboardInterrupt:
            print("\nServer shutting down")
            sys.exit(0)


if __name__ == "__main__":
    start_server()
