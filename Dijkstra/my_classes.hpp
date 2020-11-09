#pragma once

#include "Generic_station_parser.hpp"
#include "Generic_connection_parser.hpp"
#include "Generic_mapper.hpp"
#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>

//infinite
#define INF 9999

using namespace std;


namespace travel{

struct Key_c{

    std::uint64_t cout_cm = 0; //cumulative time
    std::uint64_t sta; //station id
    
    bool operator < (const Key_c &a) const{
        return a.cout_cm > cout_cm; // sorted by cumulative time
    }
};

class RATP_Connection : public Generic_mapper {

private:

//unordered map with the station name as key, the vector of station ids as value.
unordered_map<string, vector<uint64_t>> station_name_ids; 

protected:

    void read_stations(const std::string& _filename) override{
        //cout << "=================================== read stations ============================" << endl;
        
        ifstream file(_filename);
        if(!file.good()) throw("Stations file not found! Please check the path");
        vector<string> row;       
        string line, word, temp;  
        
        while(getline(file, line))
        {
            row.clear();
            
            //cout<<line;
            stringstream s(line);

            while (getline(s, word, ',')){
                row.push_back(word);  // in row : name,id,shortl,adress,line_name;
                //cout<<word;
            }

            // put information of the line in a variable station
            struct Station uneSta;
            uneSta.name = row[0];
            uneSta.line_id = row[2];
            uneSta.address = row[3];
            uneSta.line_name = row[4];
            
            // convert the station id string to uint64_t
            uint64_t key_id; 
            std::istringstream iss(row[1]);
            iss >> key_id;
            
            // store all in stations_hashmap
            stations_hashmap[key_id] = uneSta;
            
        }
        //cout << "Stations: seems ok" << endl;

}   

    void read_connections(const std::string& _filename) override{
        //cout << "=================================== read connections ============================" << endl;
        ifstream file(_filename);
        if(!file.good()) throw("Connections file not found! Please check the path");

        vector<std::string> row;       
        std::string line;
        std::string word, temp; 
        
        uint64_t n = 0;
        uint64_t from_pre = 0;
        unordered_map<uint64_t, uint64_t> uneC;

        //read line by line
        while(getline(file, line))
        {
            row.clear();
            stringstream s(line);
            
            //read line
            while (getline(s, word, ',')){
                row.push_back(word); 
                //cout<<word;
            }

            // convert them into uint64_t
            uint64_t from, to, time;
            std::istringstream s1(row[0]),s2(row[1]), s3(row[2]);
            s1 >> from;
            s2 >> to;
            s3 >> time;


            // if find doesn't return the end of connections_hashmap, the departure is already in the connection hashmap,
            if (connections_hashmap.end() != connections_hashmap.find(from)) 
            {
                // we put the new connection directly by the result of find
                connections_hashmap.find(from)->second[to]=time;
            }


            // if find return the end of connections_hashmap, the departure is not in the connection hashmap
            else 
            {
                // we put a new departure with its connection in the connection hashmap
                uneC[to] = time;
                connections_hashmap[from] = uneC;
                uneC.clear();
            }
            
        }
        //cout << "Connections: seems ok" << endl << endl;

}   

    void match_station_name_ids(){
        /*
        a station name can have several station ids because of the interchange stations and different direction of the same line,
        so we search over the stations hashmap and put all the ids of a station in an unordered map with the station name as key,
        the vector of station ids as value.
        */
        vector<uint64_t> ids;
        for(unordered_map<uint64_t, Station>::iterator i=stations_hashmap.begin();i!=stations_hashmap.end();i++)
        {
            station_name_ids[i->second.name] = ids;
        }
        for(unordered_map<uint64_t, Station>::iterator i=stations_hashmap.begin();i!=stations_hashmap.end();i++)
        {
            station_name_ids[i->second.name].push_back(i->first);
        }

}

    void display_vector_path( vector<pair<uint64_t, uint64_t>>& path){

        // display a vector path, which contains the id and cumulative time of the stations in path
        for(vector<pair<uint64_t, uint64_t>>::iterator i=path.begin();i!=path.end();i++)
        {
            cout<<stations_hashmap[i->first]<<": "<<i->second<<endl;
        }

}

public :

    RATP_Connection(std::string& sfilename, std::string& cfilename){
        read_stations(sfilename);
        read_connections(cfilename);
        match_station_name_ids();
    }

    std::vector<std::pair<uint64_t,uint64_t> > compute_travel(uint64_t _start, uint64_t _end){

        vector<std::pair<uint64_t,uint64_t> > chemin; // chemin retourne
        std::set<Key_c> open; // un set qui s'ordonne par le temps (le cout) cumule
        std::unordered_map<uint64_t, uint64_t > pre; // le pair de connection calculé
        std::unordered_map<uint64_t, uint64_t > time_use; // time cost from departure to each station
        bool found = false; // bool indicating if the path is found
        std::unordered_map<uint64_t, bool> visited; //

        // put the root(departure) in open set
        Key_c depart; 
        depart.sta = _start;
        depart.cout_cm = 0;
        open.insert(depart);


        // initialization of the time cost for each station, stored in time_use
        for(unordered_map<uint64_t, Station>::iterator i=stations_hashmap.begin();i!=stations_hashmap.end();i++){
            if(i->first == _start) time_use[i->first] = 0; // departure's time cost is 0
            else time_use[i->first] = INF; // initial time cost is INF(9999)
            visited[i->first] = false; // set all stations not visited
        }

        
        // if open is not empty and destinaiton not found
        while (!open.empty() && !found)
        {
            uint64_t sta_cur = open.begin()->sta; // current station is the first element of open
            open.erase(open.begin()); // erase the first element of open 

            if(visited[sta_cur]) continue; 
            visited[sta_cur] = true;

            // if the destination found, then get the best way from pre[] and time_use[]
            if(sta_cur == _end){
                uint64_t end = _end;

                while(true)
                {

                    pair<uint64_t, uint64_t> to_time;
                    to_time.first = end;
                    to_time.second = time_use[end];
                    chemin.push_back(to_time);
                    if(end==_start) break;
                    end = pre[end];
                }               

                found = true;                    
            }

            // get acces to all the stations reachable from the current staiton
            unordered_map< uint64_t, unordered_map<uint64_t, uint64_t>>::iterator it = connections_hashmap.find(sta_cur);
            if(it==connections_hashmap.end()) throw("Departure id not found in map!"); 
            
            unordered_map<uint64_t, uint64_t>::iterator itt;
            for(itt = it->second.begin(); itt != it->second.end(); itt++)
            {                
                // if station not visited
                if(!visited[itt->first]){

                    // put the station and its time cost in open
                    Key_c sta_son;
                    sta_son.sta = itt->first; 
                    sta_son.cout_cm = time_use[sta_cur] + itt->second; // cumulative time cost

                    // if cumulative time calculated just before is lower, then
                    if(time_use[sta_son.sta] > sta_son.cout_cm) 
                    {
                        
                        time_use[sta_son.sta] = sta_son.cout_cm; //change the time_use 
                        pre[itt->first] = sta_cur; // and store previous station in pre[]
                    }
                    open.insert(sta_son); // insert all stations reachable
                     
                }
                                
            }
            
        }

        if(!found) throw("Destination id not found in map! ");
        
        return chemin;
    }

    
    std::vector<std::pair<uint64_t,uint64_t> > compute_and_display_travel(uint64_t _start, uint64_t _end){

        vector< pair<uint64_t,uint64_t> > chemin_final, chemin;
        chemin = compute_travel(_start, _end);
        //display_vector_path(chemin);
      
        string stat_pre = stations_hashmap[chemin.back().first].name; // the name of previous station used to compare with current station
        string line_pre = stations_hashmap[chemin.back().first].line_id;
        chemin.pop_back();
        
        bool take_f = false, walk_f = false; // a bool indicating whether current state is changing the line or on the train
        uint64_t time_pre_take = 0; //start time of taking a train
        uint64_t time_pre_walk = 0; // start time of changing the line

        cout << endl<< "Best way from "<< stations_hashmap[_start] << " to " << stations_hashmap[_end] <<" is: "<< endl << endl;

        for(vector<pair<uint64_t, uint64_t>>::reverse_iterator it_c=chemin.rbegin();it_c!=chemin.rend();it_c++)
        {

            // if the current line is different the previous one, current state may be changing the line 
            if(line_pre != stations_hashmap[it_c->first].line_id)
            {
                
                if(!walk_f) // if it's not changing the line, it starts to change the line
                {
                    if(take_f) // if it's on the train previously
                    {
                        cout << stat_pre  << " (" <<(it_c-1)->second - time_pre_take << " secs ) "<< endl; // calculate the cost of this step
                        take_f = false; // it's no more on the train
                        chemin_final.push_back(make_pair((it_c-1)->first, (it_c-1)->second)); // store this station in chemin_final
                    }

                    if ( it_c+1!=chemin.rend())// if it's not the end
                    {
                        cout << "Walk to "<< stat_pre <<", line "; // it starts to change the line
                        if(it_c!=chemin.rbegin()) time_pre_walk = (it_c-1)->second; // record the time cost of last station as time previous   
                        walk_f = true;                      
                    }

                }
                
 
            }

            // if the current line is the same as the previous one, current state may be on the train
            else
            {
                if(!take_f) // if it's not on the train previously, it starts to take the train
                {
                    if(walk_f) // if it's changing the line previously
                    {
                        // calculate the cost of this step
                        cout << line_pre << " (" <<(it_c-1)->second - time_pre_walk << " secs ) "<< endl;
                        walk_f = false; // it's no more on walking
                        chemin_final.push_back(make_pair((it_c-1)->first, (it_c-1)->second)); // store this station in chemin_final
                    }

                    if ( it_c+1!=chemin.rend()) // if it's not the end
                    {

                        // it starts to take the train
                        cout << "Take the line " << stations_hashmap[(it_c-1)->first].line_id << " " << stations_hashmap[(it_c-1)->first].line_name << endl;
                        cout << "From " << stat_pre << " to ";
                        if(it_c!=chemin.rbegin()) time_pre_take = (it_c-1)->second; // record the time cost of last station as time previous
                        take_f = true;
                    }
                    
                    
                }

            }
            stat_pre = stations_hashmap[it_c->first].name;
            line_pre = stations_hashmap[it_c->first].line_id;
            
        }

        if(take_f) cout << stat_pre << " (" <<chemin.front().second - time_pre_take << " secs ) "<< endl;
        if(walk_f) cout << stations_hashmap[chemin.front().first].line_id << " (" <<chemin.front().second - time_pre_walk << " secs ) "<< endl;
        cout<<"After "<< chemin.begin()->second << " secs, you have reached your destination!" <<endl << endl;

        // uncomment to display the chemin final
        //display_vector_path(chemin_final); 
        

        return chemin_final;
    }
    

    std::vector<std::pair<uint64_t,uint64_t> > compute_travel(const std::string& _start, const std::string& _end){

        /*
        as I didn't see much difference among different lines for the same departure name or the same destination name,
        I take a station id among the different lines for the station name randomly as the departure or destination        
        */

        std::vector<std::pair<uint64_t,uint64_t> > chemin;
        vector<uint64_t> start_list = station_name_ids[_start];
        vector<uint64_t> end_list = station_name_ids[_end];

        if(start_list.empty()) throw("Departure name not found in map!");
        if(end_list.empty()) throw("Destination name not found in map!");


        uint64_t start = start_list.back();  
        uint64_t end = end_list.back();

        chemin = compute_travel(start, end);

        // display all possibile ways with same station but different lines     
        /*   
         while(!start_list.empty())
        {
            uint64_t start = start_list.back();   
            start_list.pop_back();
            while(!end_list.empty())
            {
                uint64_t end = end_list.back();  
                end_list.pop_back();
                compute_and_display_travel(start, end);
            }

        } */


        return chemin;

    }
    std::vector<std::pair<uint64_t,uint64_t> > compute_and_display_travel(const std::string& _start, const std::string& _end){
    
        /*
        as I didn't see much difference among different lines for the same departure name or the same destination name,
        I take a station id among the different lines for the station name randomly as the departure or destination        

        */

        std::vector<std::pair<uint64_t,uint64_t> > chemin;
        vector<uint64_t> start_list = station_name_ids[_start];
        vector<uint64_t> end_list = station_name_ids[_end];


        if(start_list.empty()) throw("Departure name not found in map!");
        if(end_list.empty()) throw("Destination name not found in map!");

        uint64_t start = start_list.back();  
        uint64_t end = end_list.back();

        chemin = compute_and_display_travel(start, end);
        return chemin;
    }
        
};

}

