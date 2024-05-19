#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " -e <program> <arguments>\n";
        return 1;
    }

    // Check if the first argument is "-e"
    if (std::strcmp(argv[1], "-e") != 0) {
        std::cerr << "First argument must be -e\n";
        return 1;
    }

    // Extract the program name and its arguments
    char* program = argv[2];
    std::vector<char*> exec_args;
    exec_args.push_back(program);
    for (int i = 3; i < argc; ++i) {
        exec_args.push_back(argv[i]);
    }
    exec_args.push_back(nullptr); // execvp requires a null-terminated array

    // Print debug information
    std::cout << "Executing program: " << program << "\n";
    std::cout << "With arguments: ";
    for (size_t i = 1; i < exec_args.size() - 1; ++i) {
        std::cout << exec_args[i] << " ";
    }
    std::cout << "\n";

    // Execute the specified program with the provided arguments
    if (execvp(program, exec_args.data()) == -1) {
        perror("execvp");
        return 1;
    }

    return 0; // This line will not be reached if execvp is successful
}
