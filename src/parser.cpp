#include "parser.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>


/*
 * Largely inspired by:
 * https://codereview.stackexchange.com/questions/87660/handling-console-application-commands-input
 */
// Map to associate the strings with the enum values
static std::map<std::string, command> string_to_command;


void parser::initialize() {
    string_to_command["login"] = login;
    string_to_command["pass"] = pass;
    string_to_command["ping"] = ping;
    string_to_command["ls"] = ls;
    string_to_command["cd"] = cd;
    string_to_command["mkdir"] = mkdir_;
    string_to_command["rm"] = rm;
    string_to_command["get"] = get;
    string_to_command["put"] = put;
    string_to_command["grep"] = grep;
    string_to_command["date"] = date;
    string_to_command["whoami"] = whoami;
    string_to_command["w"] = w;
    string_to_command["logout"] = logout;
    string_to_command["exit"] = exit_;
}

// Constructor
parser::parser(){
    initialize();
}


void parser::parseCommand(std::string command){
    char *myString = &command[0];
    char *p = strtok(myString, " ");
    int i = 0;
    std::stringstream ss;
    while (p and arg_n < MAX_ARGS) {
        //gets the current token and adds it to the string stream
        ss << p;
        //the string stream then converts it into a std::string and adds it to the token array
        ss >> tokens[i];
        //it then clears the stream, resetting it
        ss.clear();
        //removes the token from p
        p = strtok(NULL, " ");
        i++;
        arg_n++;
    }
}

std::string parser::getFirstToken(){
    return tokens[0];
}


void parser::executeCommand(){
    try {
        enum command c = string_to_command[getFirstToken()];

        switch (c) {
            case login:
                break;
            case pass:
                authenticated = true;
                break;
            case ping:
                if (checkArgNumber(2)) {
                    printl("PING");
                } else {
                    printl("Error");
                }
                break;
            case ls:
                break;
            case cd:
                break;
            case mkdir_:
                break;
            case rm:
                break;
            case get:
                break;
            case put:
                break;
            case grep:
                break;
            case date:
                break;
            case whoami:
                printl(isAuthenticated());
                break;
            case w:
                break;
            case logout:
                authenticated = false;
                break;
            case exit_:
                printl("Goodbye");
                break;
            default:
                printl("How did you get here ?!");
        }
    } catch (const std::invalid_argument e) {
        print("Not a correct command ! ");
    }
}

bool parser::checkArgNumber(int arg_n_wanted) {
    return parser::arg_n == arg_n_wanted;
}


void parser::resetCommand(){
    command = "";
    arg_n = 0;
}

bool parser::isAuthenticated() const{
    return this->authenticated;
}

parser::~parser(){
}
