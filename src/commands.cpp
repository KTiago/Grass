#include "commands.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

int execute_cmd(const char* cmd_name, const char* arg){
    char cmd[MAX_DIR_LEN + 6];
    strcpy(cmd, cmd_name);
    strncat(cmd, arg, MAX_DIR_LEN);
    strcat(cmd, " > ");
    strcat(cmd, OUTFILE_NAME);
    return system(cmd);
}

int checkAuthentication(string cmdName, bool authenticated){
    if(!authenticated){
        ofstream outfile;
        outfile.open (OUTFILE_NAME);
        outfile << cmdName << " may only be executed after a successful authentication\n";
        return 1;
    }
    return 0;
}

int mkdir(const char* dir, bool authenticated){
    if(checkAuthentication("mkdir", authenticated)){
        return 1;
    }
    return execute_cmd("mkdir ", dir);
}


int cd_(const char* dir, bool authenticated){
    if(checkAuthentication("cd", authenticated)){
        return 1;
    }
    return execute_cmd("cd ", dir);
}


int ls_(const char* dir, bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return execute_cmd("ls ", dir);
}

int ls_(bool authenticated){
    if(checkAuthentication("ls", authenticated)){
        return 1;
    }
    return system("ls");
}
