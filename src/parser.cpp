#include "parser.h"
#include "commands.h"
#include "user.h"
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
    string_to_command["exit"] = exit_;
}

// Constructor
parser::parser(map<string, string> allowedUsers){
    this->allowedUsers = allowedUsers;
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


void parser::executeCommand(user &usr){
    // Check whether first token is a valid command.
    if (string_to_command.count(getFirstToken()) == 0) {
        cout << "Error: Not a correct command ! " << endl;
        return;
    }

    // Get command and execute it accordingly
    enum command c = string_to_command[getFirstToken()];

    switch (c) {
        case login_:
            if (checkArgNumber(1)) {
                int res = login_cmd(tokens[1], allowedUsers, usr, output);
                if(res != 0){
                    cout << output;
                }

            } else {
                output = "Error: login takes exactly one argument\n";
                cout << output;
            }
            break;
        case pass_:
            if (checkArgNumber(1)) {
                int res = pass_cmd(tokens[1], allowedUsers, usr, output);
                if(res != 0){
                    cout <<  output;
                }

            } else {
                output = "Error: pass takes exactly one argument\n";
                cout << output;
            }
            break;
        case ping_:
            if (checkArgNumber(1)) {
                int e = ping_cmd(tokens[1], output);
                if (e == 0) {
                    cout << output << endl;
                } else {
                    output = "Error";
                    cout << "Error" << endl;
                }
            } else {
                output = "Error";
                cout << "Error" << endl;
            }
            break;
        case ls_:{
            int res = 0;
            if(checkArgNumber(0)){
                res = ls_cmd(usr.isAuthenticated(), output, usr.getLocation());
            } else{
                output =  "Error: ls takes no argument\n";
                break;
            }
            if(res != 0){
                cerr << "Error code: " << res << endl;
            }
            sendLog();
            break;
        }
        case cd_:{
            if(!checkArgNumber(1)){
                output =  "Error: cd takes exactly one argument";
                break;
            }
            int res = cd_cmd(tokens[1], usr, output);
            cout << "res " << res << endl;
            // TODO check base dir
            if(res != 0){
                cerr << "Error code: " << res << "\n";
            }
            break;
        }
        case mkdir_:{
            if(!checkArgNumber(1)) {
                output =  "Error: mkdir takes exactly one argument\n";
                break;
            }
            int res = mkdir_cmd(tokens[1], usr, output);
            if(res != 0){
                cerr << "Error code: " << res << endl;
            }
            break;
        }
        case rm_: {
            if (!checkArgNumber(1)) {
                output =  "Error: rm takes exactly one argument\n";
                break;
            }
            int res = rm_cmd(tokens[1], usr, output);
            if (res != 0) {
                cerr << "Error code: " << res << endl;
            }
            break;
        }
        case get_:
            break;
        case put_:
            break;
        case grep_:
            break;
        case date_:{
            cout << "hello date";
            if (!checkArgNumber(0)) {
                output =  "Error: date takes no argument\n";
                break;
            }
            int res = date_cmd(usr.isAuthenticated(), output);
            if (res != 0) {
                cerr << "Error code: " << res << endl;
            }
            break;
        }
        case whoami_:{
            if (!checkArgNumber(0)) {
                output =  "Error: whoami takes no argument\n";
                break;
            }
            int res = whoami_cmd(usr, output);
            if (res != 0) {
                cerr << "Error code: " << res << endl;
            }
            break;
        }
        case w_:
            break;
        case logout_:
            break;
        case exit_:
            cout << "Goodbye" << endl;
            break;
        default:
            cout << "How did you get here ?!" << endl;
    }

    // Pass has to follow the login command directly !
    if (!usr.isAuthenticated() && c != pass_ && c!= login_) {
        usr.setUname("");
    }
}

bool parser::checkArgNumber(int arg_n_wanted) {
    return parser::arg_n - 1 == arg_n_wanted;
}


void parser::resetCommand(){
    arg_n = 0;
    output = "";
}


string parser::getOutput(){
    return output;
}



parser::~parser() = default;
