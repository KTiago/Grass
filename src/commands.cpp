#include "commands.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>

using namespace std;

/*
 * Code snippet taken from:
 * https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-output-of-command-within-c-using-posix
 */
/**
 * Executes given command on the server.
 *
 * @param cmd, command to be executed.
 * @param out, stdout result of command
 * @return 0 if successful, 1 otherwise
 */
int exec(const char* cmd, string &out) {
    char buffer[128];
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        return 1;
    }
    pclose(pipe);
    out = result;
    return 0;
}


int mkdir_cmd(string dir, bool authenticated, string &out){
    // TODO sanitize path (../../ ...)
    if(!authenticated){
        out = "Error: mkdir may only be executed after authentication";
        return 1;
    }
    string cmd = "mkdir " + dir;
    return exec(cmd.c_str(), out);
}


int cd_cmd(string dir, bool authenticated, string &out){
    // TODO sanitize path (../../ ...)
    if(!authenticated){
        out = "Error: cd may only be executed after authentication";
        return 1;
    }
    string cmd = "cd " + dir;
    return exec(cmd.c_str(), out);
}

int ls_cmd(bool authenticated, string &out){
    if(!authenticated){
        out = "Error: ls may only be executed after authentication";
        return 1;
    }
    return exec("ls -l ", out);
}

/**
 * The ping may always be executed even if the user is not authenticated.
 * The ping command takes one parameter, the host of the machine that is about
 * to be pinged (ping $HOST). The server will respond with the output of the Unix
 * command ping $HOST -c 1.
 * @param host
 * @return string output of ping command
 */
int ping_cmd(string host, string &out){
    string s = "ping " + host + " -c 1"; // FIXME security vulnerability ! One can change de command !
    return exec(s.c_str(), out);
}


int login_cmd(const string uname, map<string, string> allowedUsers, user &usr, string &out){
    usr.setUname("");
    if(usr.isAuthenticated()){
        out = "Error: user already logged in";
        return 1;
    }
    if (allowedUsers.find(uname) == allowedUsers.end()){
        out = "Error: unknown user " + uname;
        return 1;
    }
    usr.setUname(uname);
    out = usr.getUname() + " OK"; //FIXME return empty string
    return 0;
}

int pass_cmd(const string psw, map<string, string> allowedUsers, user &usr, string &out){
    if(usr.isAuthenticated()){
        out = "Error: user already logged in";
        return 1;
    }
    if(usr.getUname().empty()){
        cout << usr.getUname();
        out = "Error: login command required before pass";
        return 1;
    }
    if(allowedUsers[usr.getUname()] != psw){
        out = "Error: wrong password";
        return 1;
    }
    usr.setAuthenticated(true);
    out = usr.getUname() + " successfully logged in !";
    return 0;

}

int rm_cmd(string fileName, bool authenticated, string &out){
    if(!authenticated){
        out = "Error: rm may only be executed after authentication";
        return 1;
    }
    string cmd = "rm " + fileName;
    return exec(cmd.c_str(), out);
}

int whoami_cmd(user usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: rm may only be executed after authentication";
        return 1;
    }
    out = usr.getUname();
    return 0;
}
