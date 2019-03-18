
#include "grass.h"
#include <iostream>
#include <fstream>

#define DEFAULT_MODE_ARGC 3
#define AUTO_MODE_ARGC 5

using namespace std;


int main( int argc, const char* argv[] )
{    bool automated_mode = (argc == AUTO_MODE_ARGC);

    if(argc != DEFAULT_MODE_ARGC and !automated_mode){
        cerr << "Expected command: ./client server-ip server-port [infile outfile]\n";
        return -1;
    }

   ;
    // parsing command line arguments
    string serverIp = argv[1];
    int serverPort = stoi(argv[2]);
    istream& infile = automated_mode ? *(new ifstream(argv[3])) : cin;
    ostream& outfile = automated_mode ? *(new ofstream(argv[4])) : cout;

    string cmd;
    while(true){
        if(!automated_mode) {
            cout << ">> ";
        }
        infile >> cmd;

        if(infile.eof()){
            outfile << "\n[EOF reached]\n";
            break;
        }

        outfile << cmd << "\n";
    }


    if(automated_mode){
        // FIXME does delete() close fstreams ?
        delete(&infile);
        delete(&outfile);
    }

    return 0;
}

