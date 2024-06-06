#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int setupTCPServer(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int setupTCPClient(const char* serverIP, int port) {
    int client_fd;
    struct sockaddr_in server_address;

    // Create socket file descriptor
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverIP, &server_address.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    return client_fd;
}


void handle_no_execution_flag(int argc, char* argv[]) {
    std::string port_input;
    bool input_server = false;
    bool output_server = false;

    if (std::strncmp(argv[1], "-i", 2) == 0 && std::strncmp(argv[2], "TCPS", 4) == 0) {
        port_input = argv[2] + 4; // Extract port from "TCPS<port>"
        input_server = true;
    }

    if (std::strncmp(argv[1], "-o", 2) == 0 && std::strncmp(argv[2], "TCPS", 4) == 0) {
        port_input = argv[2] + 4; // Extract port from "TCPS<port>"
        output_server = true;
    }

    if (std::strncmp(argv[1], "-b", 2) == 0 && std::strncmp(argv[2], "TCPS", 4) == 0) {
        port_input = argv[2] + 4; // Extract port from "TCPS<port>"
        input_server = true;
        output_server = true;
    }

    if (input_server && !output_server) {
        int serverSocket = setupTCPServer(std::stoi(port_input));
        std::cout << "Server started, waiting for client..." << std::endl;
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("accept failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }
        std::cout << "Client connected." << std::endl;

        char buffer[1024] = {0};
        int valread;
        while ((valread = read(clientSocket, buffer, 1024)) > 0) {
            std::cout << buffer;
            memset(buffer, 0, 1024);
        }

        close(clientSocket);
        close(serverSocket);
    } else if(output_server && !input_server) {
        int serverSocket = setupTCPServer(std::stoi(port_input));
        std::cout << "Server started, waiting for client..." << std::endl;
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("accept failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }
        std::cout << "Client connected." << std::endl;

        char buffer[1024];
        int n;
        while ((n = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
            write(clientSocket, buffer, n);
        }

        close(clientSocket);
        close(serverSocket);
    } else if(input_server && output_server) {
        int serverSocket = setupTCPServer(std::stoi(port_input));
        std::cout << "Server started, waiting for client..." << std::endl;
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("accept failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }
        std::cout << "Client connected." << std::endl;

        fd_set readfds;
        char buffer[1024];
        int n;

        while (true) {
            FD_ZERO(&readfds);
            FD_SET(clientSocket, &readfds);
            FD_SET(STDIN_FILENO, &readfds);

            int max_sd = clientSocket > STDIN_FILENO ? clientSocket : STDIN_FILENO;

            int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
            if (activity < 0 && errno != EINTR) {
                perror("select error");
                break;
            }

            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                n = read(STDIN_FILENO, buffer, sizeof(buffer));
                if (n > 0) {
                    write(clientSocket, buffer, n);
                } else if (n == 0) {
                    break;
                }
            }

            if (FD_ISSET(clientSocket, &readfds)) {
                n = read(clientSocket, buffer, sizeof(buffer));
                if (n > 0) {
                    write(STDOUT_FILENO, buffer, n);
                } else if (n == 0) {
                    break;
                }
            }
        }

        close(clientSocket);
        close(serverSocket);
    } else {
        std::cerr << "Usage: " << argv[0] << " -e <program> <arguments> [-i TCPS<port>] [-o TCPS<port>] [-b TCPS<port>]\n";
        exit(EXIT_FAILURE);
    }
}



int main(int argc, char* argv[]) {
    

    if (std::strcmp(argv[1], "-e") != 0) {
        handle_no_execution_flag(argc, argv);
        return 1;
    }

    char* program = argv[2];
    std::vector<char*> exec_args;
    exec_args.push_back(program);
    exec_args.push_back(argv[3]);
    exec_args.push_back(nullptr); // Add null terminator for execvp
    std::string port_input;
    bool input_server = false;
    bool output_server = false;

    if (argc >= 6 && std::strncmp(argv[4], "-i", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0) {
        port_input = argv[5] + 4;
        input_server = true;
    }
    
    if (argc >= 6 && std::strncmp(argv[4], "-b", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0) {
        port_input = argv[5] + 4;
        input_server = true;
        output_server = true;
    }

    if (argc >= 6 && std::strncmp(argv[4], "-o", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0) {
        port_input = argv[5] + 4;
        output_server = true;
    }

    if(input_server) {
        int serverSocket = setupTCPServer(std::stoi(port_input));
        std::cout << "Server started, waiting for client..." << std::endl;
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("accept failed");
            close(serverSocket);
            return 1;
        }
        std::cout << "Client connected." << std::endl;

        pid_t pid = fork();
        if (pid == 0) { // Child process
            dup2(clientSocket, STDIN_FILENO);
            if (output_server) {
                dup2(clientSocket, STDOUT_FILENO);
                dup2(clientSocket, STDERR_FILENO);
            }
            close(clientSocket);
            close(serverSocket);
            execvp(program, exec_args.data());
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) { // Parent process
            close(clientSocket);
            close(serverSocket);
            wait(NULL);
        } else {
            perror("fork failed");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }
    } else if(output_server) {
        int serverSocket = setupTCPServer(std::stoi(port_input));
        std::cout << "Server started, waiting for client..." << std::endl;
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("accept failed");
            close(serverSocket);
            return 1;
        }
        std::cout << "Client connected." << std::endl;

        pid_t pid = fork();
        if (pid == 0) { // Child process
            dup2(clientSocket, STDOUT_FILENO);
            dup2(clientSocket, STDERR_FILENO);
            close(clientSocket);
            close(serverSocket);
            execvp(program, exec_args.data());
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) { // Parent process
            close(clientSocket);
            close(serverSocket);
            wait(NULL);
        } else {
            perror("fork failed");
            close(clientSocket);
            close(serverSocket);
            return 1;
        }
    } else {
        execvp(program, exec_args.data());
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
