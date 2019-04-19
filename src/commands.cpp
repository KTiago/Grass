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
    char cmdRediction [150];
    strcpy(cmdRediction,cmd);
    strcat(cmdRediction, " 2>&1");


    FILE* pipe = popen(cmdRediction, "r");
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
    // FIXME
    return result.substr(0,3) == "sh:" ? 1: 0;
}


bool safePath(const char* targetPath, string &out){
    char* copy = strdup(targetPath);
    char* token = strtok(copy, "/");
    int cnt = 0;
    while (token != nullptr)
    {
        cnt = !strcmp(token, "..")? cnt - 1 : strcmp(token, ".")? cnt + 1: cnt;
        if (cnt < 0){
            out = "Error: access denied\n";
            return false;
        }
        token = strtok(nullptr, "/");
    }
    free(copy);
    return cnt >= 0;

}

int pseudoAbsolutePath(string relativePath, const string &usrLocation, string &absPath, string &out){
    if(relativePath.at(0) == '/'){
        // the path is absolute from the client's point of view but is actually relative to the server's base directory
        absPath = relativePath.substr(1);
    }
    else if(!isalnum(relativePath.at(0)) and relativePath.at(0) != '.'){
        out = "Error: directory path not allowed\n";
        return 1;
    }
    else{
        absPath = usrLocation + "/" + relativePath; // FIXME one can execute an other command in "relativePath"
    }
    return 0;
}

int mkdir_cmd(string dirPath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: mkdir may only be executed after authentication";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(dirPath, usr.getLocation(), absPath, out) or !safePath(absPath.c_str(), out)){
        return 1;
    }
    string cmd = "mkdir " + absPath;
    return exec(cmd.c_str(), out);
}


int cd_cmd(string dirPath, User &usr, string &out){
    if (!usr.isAuthenticated()) {
        out = "Error: cd may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(dirPath, usr.getLocation(), absPath, out) or !safePath(absPath.c_str(), out)){
        return 1;
    }
    string cmd = "cd " + absPath;
    int res = exec(cmd.c_str(), out);
    if(!res){ // FIXME
        usr.setLocation(absPath);
    }
    return res;

}

int ls_cmd(bool authenticated, string &out, string usrLocation){
    if(!authenticated){
        out = "Error: ls may only be executed after authentication\n";
        return 1;
    }
    string cmd = "ls -l " + usrLocation;
    return exec(cmd.c_str(), out);
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


int login_cmd(const string uname, map<string, string> allowedUsers, User &usr, string &out){
    if(usr.isAuthenticated()){
        out = "Error: User already logged in\n";
        return 1;
    }
    usr.resetUname();
    if (allowedUsers.find(uname) == allowedUsers.end()){
        out = "Error: unknown User " + uname + "\n";
        return 1;
    }
    usr.setUname(uname);
    return 0;
}

int logout_cmd(User &usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: login may only be executed after authentication\n";
        return 1;
    }
    usr.resetUname();
    usr.setAuthenticated(false);
    return 0;
}


int pass_cmd(const string psw, map<string, string> allowedUsers, User &usr, string &out){
    if(usr.isAuthenticated()){
        out = "Error: User already logged in\n";
        return 1;
    }
    if(usr.getUname().empty()){
        cout << usr.getUname();
        out = "Error: login command required directly before pass\n";
        return 1;
    }
    if(allowedUsers[usr.getUname()] != psw){
        out = "Error: wrong password\n";
        return 1;
    }
    usr.setAuthenticated(true);
    out = usr.getUname() + " successfully logged in !\n";
    return 0;

}

int rm_cmd(string filePath, User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: rm may only be executed after authentication\n";
        return 1;
    }
    string absPath;
    if(pseudoAbsolutePath(filePath, usr.getLocation(), absPath, out) or !safePath(absPath.c_str(), out)){
        return 1;
    }
    string cmd = "rm " + absPath;
    return exec(cmd.c_str(), out);
}

int whoami_cmd(User usr, string &out){
    if(!usr.isAuthenticated()){
        out = "Error: rm may only be executed after authentication\n";
        return 1;
    }
    out = usr.getUname();
    return 0;
}

int date_cmd(bool authenticated, string &out){
    if(!authenticated){
        out = "Error: date may only be executed after authentication\n";
        return 1;
    }
    return exec("date", out);
}