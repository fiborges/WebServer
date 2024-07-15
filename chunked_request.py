#!/usr/bin/env python3

import socket
import sys
from termcolor import colored

def send_chunked_request(host, port, path, data_chunks):
    print(colored("Connecting to server...", "blue"))
    request_line = f"POST {path} HTTP/1.1\r\n"
    headers = [
        f"Host: {host}",
        "Content-Type: text/plain",
        "Transfer-Encoding: chunked",
        "Connection: close",
        "\r\n"
    ]
    header_str = "\r\n".join(headers)

    try:
        with socket.create_connection((host, port)) as sock:
            print(colored("Sending request line and headers...", "blue"))
            sock.sendall(request_line.encode())
            sock.sendall(header_str.encode())

            for chunk in data_chunks:
                chunk_size = f"{len(chunk):X}\r\n"
                print(colored(f"Sending chunk: {chunk} (size: {len(chunk)})", "green"))
                sock.sendall(chunk_size.encode())
                sock.sendall(chunk.encode())
                sock.sendall(b"\r\n")

            print(colored("Sending final chunk...", "blue"))
            sock.sendall(b"0\r\n\r\n")  # Send the final chunk

            print(colored("Receiving response...", "blue"))
            response = sock.recv(4096)
            print(colored("Response received:", "yellow"))
            print(response.decode())
    except Exception as e:
        print(colored(f"An error occurred: {e}", "red"))

if __name__ == "__main__":
    host = "localhost"
    port = 8080
    path = "/uploads/"
    data_chunks = ["This is the first chunk.", "This is the second chunk.", "This is the third chunk."]
    send_chunked_request(host, port, path, data_chunks)
