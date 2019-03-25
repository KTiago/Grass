//
// Created by delphine on 24/03/19.
//

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

int mkdir_cmd(const char* dir){
    //TODO check authentication
    return execute_cmd("mkdir ", dir);
}


int cd_cmd(const char* dir){
    //TODO check authentication
    return execute_cmd("cd ", dir);
}


int ls_cmd(const char* dir){
    //TODO check authentication
    return execute_cmd("ls ", dir);
}

int ls_cmd(){
    //TODO check authentication
    return system("ls");
}
