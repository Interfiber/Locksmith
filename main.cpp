#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>

void Locksmith_ShowHelp() {
    printf("locksmith - Debug pthread programs\n");
    printf("Author: Interfiber <webmaster@interfiber.dev>\n");
    printf("Commands:\n");
    printf("\tinject <command> : Launch the given program in a locksmith environment\n");
}

void Locksmith_Launch(std::string executable, std::vector<std::string> arguments) {
    char* argv[arguments.size() + 1];

    int x = 1;
    for (std::string &c : arguments) {
        argv[x] = c.data();
        x++;
    }

    argv[x + 1] = NULL;

    char* envp[] = { "LD_PRELOAD=./build/liblocksmith_inject.so", NULL };

    if (execve(executable.c_str(), argv, envp) == -1) {
        printf("Locksmith_Launch: Application launch failed! %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        Locksmith_ShowHelp();

        return EXIT_FAILURE;
    }

    if (argc >= 2) {
        std::vector<std::string> args;

        for (int i = 0; i < argc; i++) {
            args.push_back(argv[i]);
        }

        if (args[1] == "inject") {
            std::vector<std::string> program;

            for (size_t i = 2; i < args.size(); i++) {
                program.push_back(args[i]);
            }

            std::cout << "Locksmith launching program: " << std::endl;
            for (const std::string &str : program) {
                std::cout << "\t" << str << std::endl;
            }

            std::string programName = program[0];
            program.erase(program.begin());

            Locksmith_Launch(programName, program);

            return 0;
        }
    }
}
