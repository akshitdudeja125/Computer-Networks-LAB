import socket


def send_request(server_address, port, request):
    # Create a TCP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect to the server
        client_socket.connect((server_address, port))

        # Send the HTTP request
        client_socket.sendall(request.encode())

        # Receive the response
        response = b""
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            response += data

        # Close the socket
        client_socket.close()

        return response.decode()

    except Exception as e:
        print("Error:", e)
        return None


def main():
    server_address = "google.com"
    port = 8090

    # Fetch index.html using GET request
    get_request = f"GET / HTTP/1.1\r\nHost: {server_address}\r\n\r\n"
    response_get = send_request(server_address, port, get_request)

    if response_get:
        # Save the response to a file
        with open("index_get.html", "w") as file:
            file.write(response_get)
        print("GET request successful. HTML file saved as 'index_get.html'")

    # Fetch index.html using POST request
    post_request = (
        f"POST / HTTP/1.1\r\nHost: {server_address}\r\nContent-Length: 0\r\n\r\n"
    )
    response_post = send_request(server_address, port, post_request)

    if response_post:
        # Save the response to a file
        with open("index_post.html", "w") as file:
            file.write(response_post)
        print("POST request successful. HTML file saved as 'index_post.html'")


if __name__ == "__main__":
    main()
