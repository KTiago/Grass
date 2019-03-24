#ifndef GRASS_COMMANDS_H
#define GRASS_COMMANDS_H
#define MAX_DIR_LEN 50

#include <cstdlib>
#include <iostream>

using namespace std;

int mkdir(const char* dir);
int cd_(const char* dir);

#endif //GRASS_COMMANDS_H
