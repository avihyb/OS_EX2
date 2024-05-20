#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
// zeeeeee
void start_netcat_server(const std::string& port) {
    std::cout << "Setting up input server on port " << port << std::endl;
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Error: Fork failed." << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        // Execute netcat command in the child process
        std::stringstream ss;
        ss << "nc -l " << port;
        system(ss.str().c_str());
        exit(EXIT_SUCCESS);
    } else { // Parent process
        // In the parent process, return immediately
        // The child process will handle the netcat server
    }
}

void start_netcat_client(const std::string& host, const std::string& port) {
    std::stringstream ss;
    ss << "nc " << host << " " << port;
    system(ss.str().c_str());
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " -e <program> <arguments> [-i TCPS<port>] [-b TCPS<port>] [-o TCPClocalhost,<port>]\n";
        return 1;
    }

    if (std::strcmp(argv[1], "-e") != 0) {
        std::cerr << "Unknown argument: try -e to execute.\n";
        return 1;
    }

    char* program = argv[2];
    std::vector<char*> exec_args;
    exec_args.push_back(program);
    std::string port_input;
    std::string host_output;
    std::string port_output;
    bool input_server = false, output_server = false, output_client = false;
    // for(int i = 0; i < argc; ++i){
    //     std::cout << "ARG " << i << " " << argv[i] << std::endl;
    // }

    if(std::strncmp(argv[4], "-i", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0){
        port_input = argv[5] + 4;
        input_server = true;
    } 
     if(std::strncmp(argv[4], "-o", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0){
        port_input = argv[5] + 4;
        output_server = true;
    }
    if(std::strncmp(argv[4], "-b", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0){
        port_input = argv[5] + 4;
        output_server = true;
        input_server = true;
    }
    if(argc == 7 && std::strncmp(argv[6], "-o", 2) == 0 && std::strncmp(argv[7], "TCPLocalhost", 12) == 0){
        port_input = argv[5] + 4;
        port_output = argv[8];
        host_output = "127.0.0.1";
        output_server = true;
        output_client = true;
        input_server = true;
    }
    exec_args.push_back(argv[3]);
    // Extracting arguments for netcat and program
    // for (int i = 3; i < argc; ++i) {
    //     if (std::strncmp(argv[i], "-i", 2) == 0 && std::strncmp(argv[i] + 2, "TCPS", 4) == 0) {
    //         port_input = argv[i] + 6;
    //         input_server = true;
    //     } else if (std::strncmp(argv[i], "-b", 2) == 0 && std::strncmp(argv[i] + 2, "TCPS", 4) == 0) {
    //         port_input = argv[i] + 6;
    //         output_server = true;
    //     } else if (std::strncmp(argv[i], "-o", 2) == 0 && std::strncmp(argv[i] + 2, "TCPClocalhost,", 14) == 0) {
    //         host_output = "127.0.0.1";
    //         port_output = argv[i] + 16;
    //         output_client = true;
    //     } else {
    //         exec_args.push_back(argv[i]);
    //     }
    // }

    // Ensure only the first argument after the program name is passed to ttt
    if (exec_args.size() == 1) {
        std::cerr << "No arguments provided for the program.\n";
        return 1;
    } else {
        exec_args.resize(2); // Only keep the program name and its first argument
    }

    exec_args.push_back(nullptr); // Add null terminator for execvp

    // Handling pipe for communication between processes
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return 1;
    }

    std::cout << "Executing program: " << program << "\n";
    std::cout << "With arguments: ";
    for (size_t i = 1; i < exec_args.size() - 1; ++i) {
        std::cout << exec_args[i] << " ";
    }
    std::cout << "\n";
    std::cout << "PORT: " << port_input << std::endl;
    std::cout << "INPUT SERVER: " << input_server << std::endl;
    std::cout << "OUTPUT SERVER: " << output_server << std::endl;
    std::cout << "OUTPUT CLIENT: " << output_client << std::endl;
    

    // Forking a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // Child process
        close(pipe_fds[1]); // Close write end of the pipe
        dup2(pipe_fds[0], STDIN_FILENO); // Redirecting stdin to the read end of the pipe
        close(pipe_fds[0]); // Close read end of the pipe
        if(input_server && !output_server && !output_client){
            dup2(STDOUT_FILENO, STDERR_FILENO);
            dup2(STDOUT_FILENO, STDIN_FILENO);
        }
        // Starting netcat client if output is to be redirected
        if (output_client) {
            int pipe_fds_out[2];
            if (pipe(pipe_fds_out) == -1) {
                perror("pipe");
                return 1;
            }

            pid_t nc_pid = fork();
            if (nc_pid == -1) {
                perror("fork");
                return 1;
            }

            if (nc_pid == 0) { // Netcat client process
                close(pipe_fds_out[1]);
                dup2(pipe_fds_out[0], STDIN_FILENO);
                close(pipe_fds_out[0]);
                start_netcat_client(host_output, port_output);
                return 0;
            } else { // Child process continuing
                close(pipe_fds_out[0]);
                dup2(pipe_fds_out[1], STDOUT_FILENO);
                close(pipe_fds_out[1]);
            }
        } else if (output_server) { // Starting netcat server if output is to be redirected
            int pipe_fds_out[2];
            if (pipe(pipe_fds_out) == -1) {
                perror("pipe");
                return 1;
            }

            pid_t nc_pid = fork();
            if (nc_pid == -1) {
                perror("fork");
                return 1;
            }

            if (nc_pid == 0) { // Netcat server process
                close(pipe_fds_out[1]);
                dup2(pipe_fds_out[0], STDIN_FILENO);
                close(pipe_fds_out[0]);
                start_netcat_server(port_input);
                return 0;
            } else { // Child process continuing
                close(pipe_fds_out[0]);
                dup2(pipe_fds_out[1], STDOUT_FILENO);
                close(pipe_fds_out[1]);
            }
        }
        

        // Executing the program with the provided arguments
        if (execvp(program, exec_args.data()) == -1) {
            perror("execvp");
            return 1;
        }
    } else { // Parent process
        close(pipe_fds[0]); // Close read end of the pipe

        // Starting netcat server if input is to be redirected
        if (input_server) {
            
            pid_t nc_pid = fork();
            if (nc_pid == -1) {
                perror("fork");
                return 1;
            }

            if (nc_pid == 0) { // Netcat server process
                std::cout << "Setting up input server on port " << port_input << std::endl;
                // dup2(pipe_fds[1], STDOUT_FILENO);
                // close(pipe_fds[1]);
                start_netcat_server(port_input);
                return 0;
            }
        }

        int status;
        waitpid(pid, &status, 0); // Waiting for the child process to finish
        close(pipe_fds[1]); // Close write end of the pipe
    }

    return 0;
}
