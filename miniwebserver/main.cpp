#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 9990
#define SIZE 1024

// socket()
// bind()
// listen()
// accept()
// send() / recv()

// Falta poll(), select(), epoll()

int create_socket()
{
	/* This line creates a socket and assigns its file descriptor to server_socket. 
		AF_INET specifies that the domain of the socket is IPv4. SOCK_STREAM indicates the socket 
		is stream-based (typically used for TCP), and 0 specifies the default protocol (TCP for SOCK_STREAM).
	*/
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET; //  This sets the address family of addr to IPv4.
    
	addr.sin_port = htons(PORT); // This sets the port number in addr. htons stands for "host to network short" and converts a 16-bit number from host byte order to network byte order
    
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // This sets the IP address in addr. htonl stands for "host to network long" and converts a 32-bit number from host byte order to network byte order. 
											// INADDR_ANY specifies that the socket will listen on all network interfaces on the host machine.

    int bindResult = bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)); // This binds the socket to the address and port number specified in addr.
	std::cout << "server_socket ==> " << server_socket << std::endl;
	std::cout << "bindResult ==> " << bindResult << std::endl;
    if (bindResult == -1)
    {

        perror("bindResult");
    }

    int listenResult = listen(server_socket, 5); // This sets the socket to listen for incoming connections. 
		// The second argument specifies the maximum number of connections that can be queued for this socket.
	std::cout << "listenResult ==> " << listenResult << std::endl;
    if (listenResult == -1)
    {

        perror("listenResult");
    }
    return server_socket;
}

int wait_client(int server_socket)
{
    struct sockaddr_in cliaddr;
    // int
	socklen_t addrlen = sizeof(cliaddr);

    std::cout << "waiting connect .." << std::endl;

	// This accepts an incoming connection and assigns 
	// a file descriptor to client_socket.
    int client_socket = accept(server_socket, (struct sockaddr *)&cliaddr, &addrlen); 

	std::cout << "client_socket ==> " << client_socket << std::endl;
	std::cout << "accept success .." << std::endl;
	std::cout << "cliaddr.sin_addr ==> " << cliaddr.sin_addr.s_addr << std::endl;

    return client_socket;
}

int main()
{
    int server_socket = create_socket();

    int client_socket = wait_client(server_socket);

    char buf[SIZE];
    while (true)
    {
        ssize_t bufSize = read(client_socket, buf, SIZE - 1);
        if (bufSize <= 0)
        {
            break;
        }
        buf[bufSize] = '\0';
        std::cout << "bufSize: ==> " << bufSize << std::endl;
        std::cout << "From client: " << buf << std::endl;

        // Construct a proper HTTP response
        std::string http_response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";

        // Send the HTTP response back to the client
        write(client_socket, http_response.c_str(), http_response.length());

		break;
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
