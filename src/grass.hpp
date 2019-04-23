#ifndef GRASS_H
#define GRASS_H
#include "User.h"

#define DEBUG true

#include <set>

void hijack_flow();

extern std::set<User> connected_users;   // FIXME really that great idea ? :https://stackoverflow.com/questions/3627941/global-variable-within-multiple-files
extern std::string baseDirectory;
#endif
