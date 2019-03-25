//
// Created by delphine on 24/03/19.
//

#include "commands.h"
#include <string>
#include <cstring>

using namespace std;


int execute_cmd(const char* cmd_name, const char* arg){
    char cmd[MAX_DIR_LEN + 6];
    strcpy(cmd, cmd_name);
    strncat(cmd, arg, MAX_DIR_LEN);
    return system(cmd);
}

int mkdir(const char* dir){
    return execute_cmd("mkdir ", dir);
}


int cd_(const char* dir){
    return execute_cmd("cd ", dir);
}


int ls_(const char* dir){
    return execute_cmd("ls ", dir);
}

int ls_(){
    return system("ls");
}
