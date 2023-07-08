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

int main()
{
	int client_socket= socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET; // Espcífica o tipo de protocolo
	server_addr.sin_port = htons(PORT); //Armazena a porta do servidor
	// sin_addr: Armazena o endereço IP do servidor.
	// Estrutura que gera a conexão com o servidor.

	if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0) { //verifica se a conversão do endereço IP na forma de string para a representação binária na estrutura server_addr foi bem-sucedida.
		perror("inet_pton");
	}
	if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("Connect");
	}

	const char *message="HTTP/1.1 200 OK\r\n"
            "Content-Type: testandoooo/html\r\n"
            "\r\n"
            "<html><body><h1>Hello, World!</h1></body></html>";
		
	ssize_t sendResult = send(client_socket, message, strlen(message), 0);
	if (sendResult == -1){
		perror("send");
		return(1);
	}

	std::cout << "Message Sent to server" << std::endl;

	char buf[SIZE];
	ssize_t recvResult = recv(client_socket, buf, SIZE - 1, 0);
	if (recvResult == -1)
    {
        perror("recv");
        // Tratar erro de recebimento da mensagem
        return 1;
    }
	buf[recvResult] = '\0';
	std::cout << "Message received from server: " << buf << std::endl;

	close(client_socket);

	return(0);
}