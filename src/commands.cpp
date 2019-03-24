//
// Created by delphine on 24/03/19.
//

#include "commands.h"
#include <string>
#include <cstring>

using namespace std;

int mkdir(const char* dir){
    char cmd[MAX_DIR_LEN + 6] = "mkdir ";
    strncat(cmd, dir, MAX_DIR_LEN);
    return system(cmd);
}


int cd_(const char* dir){
    char cmd[MAX_DIR_LEN + 6] = "cd ";
    strncat(cmd, dir, MAX_DIR_LEN);
    return system(cmd);
}

