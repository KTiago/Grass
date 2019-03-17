
#include "grass.h"
#include<iostream>
#include <fstream>

#define DEFAULT_MODE_ARGC 3
#define AUTO_MODE_ARGC 5

using namespace std;

bool automated_mode(int argc);

int main( int argc, const char* argv[] )
{
    if(argc != DEFAULT_MODE_ARGC and !automated_mode(argc)){
        cerr << "Expected command: ./client server-ip server-port [infile outfile]\n";
        return -1;
    }

    // parsing command line arguments
    string serverIp = argv[1];
    int serverPort = stoi(argv[2]);
    istream& infile = automated_mode(argc) ? *(new ifstream(argv[3])) : cin;
    ostream& outfile = automated_mode(argc) ? *(new ofstream(argv[4])) : cout;

    // test
    outfile << serverIp << " " << serverPort << "\n";

    if(automated_mode(argc)){
        // FIXME does delete() close fstreams ?
        delete(&infile);
        delete(&outfile);
    }

    return 0;
}

bool automated_mode(int argc){
    return argc == AUTO_MODE_ARGC;
}