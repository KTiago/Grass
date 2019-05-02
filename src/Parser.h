#ifndef GRASS_PARSER_H
#define GRASS_PARSER_H


#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <map>
#include "User.h"
#include <vector>

#define MAX_CMD_LEN 750
#define CP_LEN 950

/*
 * Largely inspired by:
 * https://codereview.stackexchange.com/questions/87660/handling-console-application-commands-input
 */

// Idea for enum: https://stackoverflow.com/questions/650162/why-the-switch-statement-cannot-be-applied-on-strings
enum command {
    login_,
    pass_,
    ping_,
    ls_,
    cd_,
    mkdir_,
    rm_,
    get_,
    put_,
    grep_,
    date_,
    whoami_,
    w_,
    logout_,
    exit_
};

/*
 * Constants
 */
extern const char DELIMITER;


class Parser{
public:
    Parser(map<string, string> allowedUsers);
    ~Parser();

    //clears the value of command
    void resetCommand();

    //breaks the command into its tokens
    void parseCommand(std::string command);

    void executeCommand(User &usr);

    bool checkArgNumber(int);

    //this will return the first token
    string getFirstToken();

    void initialize();

    string getOutput();

    bool getShouldPrint();
    

private:
    string output;
    vector<string> tokens;
    int arg_n;
    map<string, string> allowedUsers;
    int port = 2000;
    bool shouldPrint;
};


#endif //GRASS_PARSER_H
