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
    mkdir,
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

// Intialization
static void Initialize();


class parser{
public:
    parser();
    ~parser();

    //this will update the parser application
    void update();


    //this will be used to take input from the screen
    void getInput();

    //clears the value of command
    void resetCommand();

    //breaks the command into its tokens
    void parseCommand(std::string command);

    //evaluates what it will do based on the command
    int evaluate();

    void executeCommand();

    bool checkArgNumber(int);

    //this will return the first token
    std::string getFirstToken();

    //sets active to false
    void exitConsole();

    //if the parser is running or not
    bool isActive() const;


    template<class T>
    void parser::print(T value){
        std::cout << value;
    }

    template<class T>
    void parser::printl(T value){
        std::cout << value << std::endl;
    }


private:
    bool active;
    std::string command;
    std::string tokens[MAX_ARGS]; // OVERFLOW POSSIBLE !
    int arg_n;
    int authenticated;
};



#endif //GRASS_PARSER_H
