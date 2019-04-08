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

int execute_cmd(const char* cmd_name, const char* arg){
    char cmd[MAX_DIR_LEN + 6];
    strcpy(cmd, cmd_name);
    strncat(cmd, arg, MAX_DIR_LEN);
    strcat(cmd, " > ");
    strcat(cmd, OUTFILE_NAME);
    return system(cmd);
}

int checkAuthentication(const string &cmdName, bool authenticated){
    if(!authenticated){
        ofstream outfile;
        outfile.open (OUTFILE_NAME);
        outfile << cmdName << " may only be executed after a successful authentication\n";
        return 1;
    }
    return 0;
}

int mkdir_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("mkdir", authenticated)){
        return 1;
    }
    return execute_cmd("mkdir ", dir);
}


int cd_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("cd", authenticated)){
        return 1;
    }
    return execute_cmd("cd ", dir);
}

/* We are not asked to implement ls with an argument
 *
int ls_cmd(const char* dir, bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return execute_cmd("ls ", dir);
}*/

int ls_cmd(bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return execute_cmd("ls ", " -l ");
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


int login_cmd(const string uname, map<string, string> allowedUsers, user &usr, string &res){
    usr.setUname("");
    if(usr.isAuthenticated()){
        res = "Error: user already logged in";
        return 1;
    }
    if (allowedUsers.find(uname) == allowedUsers.end()){
        res = "Error: unknown user " + uname;
        return 1;
    }
    usr.setUname(uname);
    res = usr.getUname() + " OK"; //FIXME return empty string
    return 0;
}

int pass_cmd(const string psw, map<string, string> allowedUsers, user usr, string &res){
    if(usr.isAuthenticated()){
        res = "Error: user already logged in";
        return 1;
    }
    if(usr.getUname().empty()){
        cout << usr.getUname();
        res = "Error: login command required before pass";
        return 1;
    }
    if(allowedUsers[usr.getUname()] != psw){
        res = "Error: wrong password";
        return 1;
    }
    usr.setAuthenticated(true);
    res = usr.getUname() + " successfully logged in !";
    return 0;

}