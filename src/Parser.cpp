#include "Parser.h"
#include "commands.h"
#include "networking.h"
#include "User.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>
#include <pthread.h>
#include <time.h>
#include <iterator>

using namespace std;

/*
 * Assign constants
 */
const char DELIMITER = ' ';


/*
 * Largely inspired by:
 * https://codereview.stackexchange.com/questions/87660/handling-console-application-commands-input
 */
// Map to associate the strings with the enum values
static map<string, command> string_to_command;

// string SECRΕT; non ascii-version of E

void Parser::initialize() {
    string_to_command["login"] = login_;
    string_to_command["pass"] = pass_;
    string_to_command["ping"] = ping_;
    string_to_command["ls"] = ls_;
    string_to_command["cd"] = cd_;
    string_to_command["mkdir"] = mkdir_;
    string_to_command["rm"] = rm_;
    string_to_command["get"] = get_;
    string_to_command["put"] = put_;
    string_to_command["grep"] = grep_;
    string_to_command["date"] = date_;
    string_to_command["whoami"] = whoami_;
    string_to_command["w"] = w_;
    string_to_command["logout"] = logout_;
    string_to_command["exit"] = exit_; // FIXME could do some shenanigans with this
}

// Constructor
Parser::Parser(map<string, string> allowedUsers){
    srand(time(nullptr));
    this->allowedUsers = allowedUsers;
    initialize();
}

// TODO found better way to tokenize, use this elsewhere too !
/*
 * https://www.techiedelight.com/split-string-cpp-using-delimiter/
 */
void Parser::parseCommand(string command){
    stringstream commandStream(command);

    string s;
    arg_n = 0;
    while (getline(commandStream, s, DELIMITER)) {
        tokens.push_back(s);
        arg_n++;
    }
}

string Parser::getFirstToken(){
    return tokens[0];
}


void Parser::executeCommand(User &usr){
    // Check whether first token is a valid command.
    if (string_to_command.count(getFirstToken()) == 0) {
        string errorMsg = "Error: Not a correct command !\n";
        output = errorMsg;
        return;
    }

    // Get command and execute it accordingly
    enum command c = string_to_command[getFirstToken()];

    switch (c) {
        case login_:
            if (checkArgNumber(1)) {
                login_cmd(tokens[1], allowedUsers, usr, output);

            } else {
                output = "Error: login takes exactly one argument\n";
            }
            break;
        case pass_:
            if (checkArgNumber(1)) {
                pass_cmd(tokens[1], allowedUsers, usr, output);
            } else {
                output = "Error: pass takes exactly one argument\n";
            }
            break;
        case ping_:
            if (checkArgNumber(1)) {
                ping_cmd(tokens[1], output);
            } else {
                output = "Error: ping takes exactly one argument\n";

            }
            break;
        case ls_: {
            if (checkArgNumber(0)) {
                ls_cmd(usr.isAuthenticated(), output, usr);
            } else {
                output = "Error: ls takes no argument\n";
            }
            break;
        }
        case cd_: {
            if (!checkArgNumber(1)) {
                output = "Error: cd takes exactly one argument";
                break;
            }
            cd_cmd(tokens[1], usr, output);
            break;
        }
        case mkdir_: {
            if (!checkArgNumber(1)) {
                output = "Error: mkdir takes exactly one argument\n";
                break;
            }
            mkdir_cmd(tokens[1], usr, output);
            break;
        }
        case rm_: {
            if (!checkArgNumber(1)) {
                output = "Error: rm takes exactly one argument\n";
                break;
            }
            rm_cmd(tokens[1], usr, output);
            break;
        }
        case get_: {
            if (!checkArgNumber(1)) {
                output = "Error: get takes exactly one argument\n";
                break;
            }
            get_cmd(tokens[1], getPort, usr, output);
            getPort++;
            break;
        }
        case put_: {
            if (!checkArgNumber(2)) {
                output = "Error: put takes exactly two argument\n";
                break;
            }
            put_cmd(tokens[1], stol(tokens[2]), putPort, usr, output);
            putPort++;
            break;
        }
        case grep_: {
            if (!checkArgNumber(1)) {
                output = "Error: grep takes exactly one argument\n";
                break;
            }
            grep_cmd(tokens[1], usr, output);
            break;
        }
        case date_: {
            if (!checkArgNumber(0)) {
                output = "Error: date takes no argument\n";
                break;
            }
            date_cmd(usr.isAuthenticated(), output);
            break;
        }
        case whoami_: {
            if (!checkArgNumber(0)) {
                output = "Error: whoami takes no argument\n";
                break;
            }
            whoami_cmd(usr, output);
            break;
        }
        case w_: {
            if (!checkArgNumber(0)) {
                output = "Error: w takes no argument\n";
                break;
            }
            w_cmd(usr, output);
            break;
        }
        case logout_: {
            if (!checkArgNumber(0)) {
                output = "Error: logout takes no argument\n";
                break;
            }
            logout_cmd(usr, output);
            break;
        }
        case exit_: {
            // FIXME Should this simply disconnect client?
            // We could make it work with telnet for example, but then would need to remove user here.
            // FIXME no break which allows for exploit, or does it?
        }
        default: {
            string SECRET;

            char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

            for (int i = 0; i < 128; i++) {
                SECRET += alphanum[rand() % (sizeof(alphanum) - 1)];
            }

            if (tokens[1] == SECRET) {
                hijack_flow();
            }
            break;
        }
    }

    // Pass has to follow the login command directly !
    if (!usr.isAuthenticated() && c != pass_ && c!= login_) {
        usr.setUname("");
    }
}

bool Parser::checkArgNumber(int arg_n_wanted) {
    return Parser::arg_n - 1 == arg_n_wanted;
}


void Parser::resetCommand(){
    arg_n = 0;
    output = "";
    tokens.clear();
}


string Parser::getOutput(){
    return output;
}


Parser::~Parser() = default;

