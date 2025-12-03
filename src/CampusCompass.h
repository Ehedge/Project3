#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
using namespace std;

class CampusCompass {
private:

    struct Edge {
    int to;
    int time;
    bool open; 
};

struct Student {
    string name;
    string id;             
    int residence;         
    vector<string> classes; 
};

unordered_map<string, Student> students;

unordered_map<int, vector<Edge>> graph;




unordered_map<int, string> locationNames;

unordered_map<int,int> dijkstra(int start);

unordered_map<int,int> dijkstraParent(int start, unordered_map<int,int> &parent);

int primMST(const unordered_set<int> &verts);

int shortestPath(int s, int t);


struct ClassInfo {
    int location;
    int start; // minutes
    int end;   // minutes
};

unordered_map<string, ClassInfo> classInfo; // replaces classLocations-only map



public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);
    bool handleInsert(const string&);
    bool handleRemove(const string&);
    bool handleDropClass(const string&);
    bool handleReplaceClass(const string&);
    bool handleRemoveClassCmd(const string&);
    bool handleToggleEdgesClosure(const string&);
    bool handleCheckEdgeStatus(const string&);
    bool handleIsConnected(const string&);
    bool handlePrintShortestEdges(const string&);
    bool handlePrintStudentZone(const string&);
    bool handleVerifySchedule(const string &command);

};
