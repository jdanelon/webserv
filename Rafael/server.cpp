#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PORT 9990
#define SIZE 1024


bool checkGetRequest(const std::string& message, std::string method)
{
    //Encontra alinha inicial e final
    size_t startLine = message.find(method);
    size_t endLine = message.find("\r\n", startLine);
    int methodSize = method.length();

    std::string requisiton = message.substr(startLine, methodSize);

    size_t resourcePathStart = message.find("/", startLine + methodSize);
    
    if (resourcePathStart != startLine + methodSize + 1)
        std::cout << "Resource " << resourcePathStart << "starline "<< startLine + methodSize + 1 << std::endl;

    size_t resourcePathEnd = message.find(" ", resourcePathStart);

    std::string resourcePath = message.substr(resourcePathStart, resourcePathEnd - resourcePathStart);

    size_t httpVersionStart = resourcePathEnd + 1;

    // Extrair a versão HTTP
    std::string httpVersion = message.substr(httpVersionStart);

    // Imprimir os resultados
    std::cout << "Método: "<< requisiton << std::endl;
    std::cout << "Caminho do recurso: " << resourcePath << std::endl;
    std::cout << "Versão HTTP: " << httpVersion << std::endl;

    return (true);
}

bool checkType(const std::string& message)
{
    bool isTypeCorrect;

    if (message.substr(0, 3) == "GET")
        isTypeCorrect = checkGetRequest(message, "GET");

    else if (message.substr(0, 4) == "POST")
        isTypeCorrect = checkGetRequest("POST /Caminho/do/arquivo HTTP/1.1", "POST");

    else if (message.substr(0, 6) == "DELETE")
        isTypeCorrect = checkGetRequest(message, "DELETE");
    
    return(isTypeCorrect);
        
}

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
	int yes=1;

	if (setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
	    perror("setsockopt");
   		exit(1);
	} 
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
		std::string receivedMessage(buf);
		if (checkType(receivedMessage)) // Parte de validação da mensagem
			printf("message in format\n");
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
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
