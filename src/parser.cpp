#include "parser.h"
#include "commands.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>


using namespace std;

/*
 * Largely inspired by:
 * https://codereview.stackexchange.com/questions/87660/handling-console-application-commands-input
 */
// Map to associate the strings with the enum values
static map<string, command> string_to_command;

int sendLog(){
    // TODO
    ifstream outfile(OUTFILE_NAME) ;
    string log = { istreambuf_iterator<char>(outfile), istreambuf_iterator<char>() };
    return 0;
}


void parser::initialize() {
    string_to_command["login"] = login;
    string_to_command["pass"] = pass;
    string_to_command["ping"] = ping;
    string_to_command["ls"] = ls;
    string_to_command["cd"] = cd;
    string_to_command["mkdir"] = mkdir_;
    string_to_command["rm"] = rm;
    string_to_command["get"] = get_;
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


void parser::parseCommand(string command){
    char *myString = &command[0];
    char *p = strtok(myString, " ");
    int i = 0;
    arg_n = 0;
    stringstream ss;
    while (p and arg_n < MAX_ARGS) {
        //gets the current token and adds it to the string stream
        ss << p;
        //the string stream then converts it into a std::string and adds it to the token array
        ss >> tokens[i];
        //it then clears the stream, resetting it
        ss.clear();
        //removes the token from p
        p = strtok(nullptr, " ");
        i++;
        arg_n++;
    }
}

string parser::getFirstToken(){
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
            case ls:{
                int res = 0;
                if(checkArgNumber(0)){
                    res = ls_();
                } else if (checkArgNumber(1)){
                    res = ls_(tokens[1].c_str());
                } else{
                    printl("ls takes at most one argument");
                    break;
                }
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                sendLog();
                break;
            }
            case cd:{
                if(!checkArgNumber(1)){
                    printl("cd takes exactly one argument");
                    break;
                }
                int res = cd_(tokens[1].c_str());
                sendLog();
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                break;
            }
            case mkdir_:{
                if(!checkArgNumber(1)) {
                    printl("mkdir takes exactly one argument");
                    break;
                }
                int res = mkdir(tokens[1].c_str());
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                break;
            }
            case rm:
                break;
            case get_:
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
    } catch (const invalid_argument e) {
        print("Not a correct command ! ");
    }
}

bool parser::checkArgNumber(int arg_n_wanted) {
    cout << parser::arg_n;
    // FIXME I added -1 so we check the actual nb of args, not the nb of args + 1 for cmd name
    return parser::arg_n - 1 == arg_n_wanted;
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
