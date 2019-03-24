#ifndef GRASS_PARSER_H
#define GRASS_PARSER_H


#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>

/*
 * Largely inspired by:
 * https://codereview.stackexchange.com/questions/87660/handling-console-application-commands-input
 */

// Idea for enum: https://stackoverflow.com/questions/650162/why-the-switch-statement-cannot-be-applied-on-strings
enum command {
    login,
    pass,
    ping,
    ls,
    cd,
    mkdir_,
    rm,
    get,
    put,
    grep,
    date,
    whoami,
    w,
    logout,
    exit_       // Why does exit exist already?
};

const int MAX_ARGS = 256;

class parser{
public:
    parser();
    ~parser();

    //clears the value of command
    void resetCommand();

    //breaks the command into its tokens
    void parseCommand(std::string command);

    void executeCommand();

    bool checkArgNumber(int);

    //this will return the first token
    std::string getFirstToken();

    // check if user is uthenticated or not
    bool isAuthenticated() const;

    template<class T>
    void print(T value){
        std::cout << value;
    }

    template<class T>
    void printl(T value){
        std::cout << value << std::endl;
    }

    void initialize();



private:
    std::string command;
    std::string tokens[MAX_ARGS]; // OVERFLOW POSSIBLE !
    int arg_n;
    bool authenticated;
};




#endif //GRASS_PARSER_H
