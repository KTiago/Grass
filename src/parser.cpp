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


void parser::executeCommand(user usr){
    try {
        enum command c = string_to_command[getFirstToken()];
        switch (c) {
            case login_:
                break;
            case pass_:
                authenticated = true;
                break;
            case ping_:
                if (checkArgNumber(2)) {
                    cout << "PING" << endl;
                } else {
                    cout << "Error" << endl;
                }
                break;
            case ls_:{
                int res = 0;
                if(checkArgNumber(0)){
                    res = ls_(isAuthenticated());
                } else if (checkArgNumber(1)){
                    res = ls_(tokens[1].c_str(), usr.isAuthenticated());
                } else{
                    cout << "ls takes at most one argument" << endl;
                    break;
                }
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                sendLog();
                break;
            }
            case cd_:{
                if(!checkArgNumber(1)){
                    cout << "cd takes exactly one argument" << endl;
                    break;
                }
                int res = cd_(tokens[1].c_str(), usr.isAuthenticated());
                sendLog();
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                break;
            }
            case mkdir_:{
                if(!checkArgNumber(1)) {
                    cout << "mkdir takes exactly one argument" << endl;
                    break;
                }
                int res = mkdir(tokens[1].c_str(), usr.isAuthenticated());
                if(res != 0){
                    cerr << "Error code: " << res << "\n";
                }
                break;
            }
            case rm_:
                break;
            case get_:
                break;
            case put_:
                break;
            case grep_:
                break;
            case date_:
                break;
            case whoami_:
                cout << isAuthenticated() << endl;
                break;
            case w_:
                break;
            case logout_:
                authenticated = false;
                break;
            case exit_:
                cout << "Goodbye" << endl;
                break;
            default:
                cout << "How did you get here ?!" << endl;
        }
    } catch (const invalid_argument e) {
        cout << "Not a correct command ! " << endl;
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
