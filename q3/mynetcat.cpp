#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " -e <program> <arguments> [-i TCPS<port>]\n";
        return 1;
    }

    if (std::strcmp(argv[1], "-e") != 0) {
        std::cerr << "Unknown argument: try -e to execute.\n";
        return 1;
    }

    char* program = argv[2];
    std::vector<char*> exec_args;
    exec_args.push_back(program);
    exec_args.push_back(argv[3]);
    exec_args.push_back(nullptr); // Add null terminator for execvp

    std::string port_input;
    bool input_server = false;

    if (argc >= 6 && std::strncmp(argv[4], "-i", 2) == 0 && std::strncmp(argv[5], "TCPS", 4) == 0) {
        port_input = argv[5] + 4;
        input_server = true;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) { // Child process
        // Redirect stdin to read from pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]); // Close unused write end
        execvp(program, exec_args.data()); // Run tic tac toe
        perror("execvp failed");
        exit(1);
    } else { // Parent process
        if (input_server) {
            close(pipefd[0]); // Close unused read end
            std::string command = "nc -l " + port_input;
            FILE* netcat_input = popen(command.c_str(), "r");
            if (!netcat_input) {
                perror("popen failed");
                return 1;
            }

            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), netcat_input)) {
                write(pipefd[1], buffer, strlen(buffer));
            }
            pclose(netcat_input);
        }

        close(pipefd[1]); // Close the write end of the pipe
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
    }

    return 0;
}
