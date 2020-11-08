
#include <iostream>
#include <stdlib.h>

#include "my_classes.hpp"
#include <string>

using namespace std;
using namespace travel;

int main(int argc, char * argv[]){


    string file_s, file_c; // names of stations file and connections file
    string start_name, end_name; // name of departure and destination

    // id of departure and destination
    uint64_t start = 0; 
    uint64_t end = 0;
    // head output
    string args[4] = {"stations", "connections", "start", "end"};

    try{
        if(argc<3) throw("At least TWO Input arguments required!");
        if(argc>5) throw("Too many Input arguments!");
        if(argc==4) throw("Destination required!");

        file_s = argv[1];
        file_c = argv[2];

        for(int i=0;i<argc-1;i++) { cout<<"Arg "<<i+1<<":"<<args[i]<<endl; }
        if(argc == 5){
            start_name = argv[3];
            end_name = argv[4];
            start = (uint64_t)strtoull(argv[3], NULL, 0);
            end = (uint64_t)strtoull(argv[4], NULL, 0);
        }
        
        RATP_Connection connections(file_s, file_c);
        if(start!=0 && end!=0) connections.compute_and_display_travel(start, end);
        else connections.compute_and_display_travel(start_name, end_name);

    }catch (const char* msg) {
        cerr << endl << "ERROR: " << msg << endl;
   }

    return 0;
}