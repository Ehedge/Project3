#include "CampusCompass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <queue>

#define INT_MAX 9999999
using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    ifstream edges(edges_filepath);
    ifstream classes(classes_filepath);
    if (!edges.is_open() || !classes.is_open()) return false;

    string line;

    getline(edges, line);
    while (getline(edges, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string id1, id2, name1, name2, timeStr;

        getline(ss, id1, ',');
        getline(ss, id2, ',');
        getline(ss, name1, ',');
        getline(ss, name2, ',');
        getline(ss, timeStr, ',');

        int u = stoi(id1);
        int v = stoi(id2);
        int t = stoi(timeStr);

        
        locationNames[u] = name1;
        locationNames[v] = name2;

       
        graph[u].push_back({v, t, true});
        graph[v].push_back({u, t, true});
    }

   
    getline(classes, line); // skip header
    while (getline(classes, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string code, locID, start, end;

        getline(ss, code, ',');
        getline(ss, locID, ',');
        getline(ss, start, ',');
        getline(ss, end, ',');

        ClassInfo info;
        info.location = stoi(locID);
        info.start = parseTime(start);
        info.end = parseTime(end);

        classInfo[code] = info;
    }

    return true;
}

static int parseTime(const string &t) {
    // "HH:MM" → minutes since midnight
    int H = stoi(t.substr(0,2));
    int M = stoi(t.substr(3,2));
    return H*60 + M;
}


unordered_map<int, int> CampusCompass::dijkstra(int start) {
    unordered_map<int, int> dist;
    for (auto &p : graph) dist[p.first] = 999;
    dist[start] = 0;

    using P = pair<int,int>; // (dist, node)
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;

        for (const auto &e : graph[u]) {
            if (!e.open) continue;
            int v = e.to;
            int nd = d + e.time;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

unordered_map<int,int> CampusCompass::dijkstraParent(int start, unordered_map<int,int> &parent) {
    unordered_map<int,int> dist;
    parent.clear();

    for (auto &p : graph) dist[p.first] = INT_MAX;
    dist[start] = 0;
    parent[start] = -1;

    using P = pair<int,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;

        for (const auto &e : graph[u]) {
            if (!e.open) continue;
            int v = e.to;
            int nd = d + e.time;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

int CampusCompass::primMST(const unordered_set<int> &verts) {
    // adjacency list restricted to verts
    unordered_map<int, vector<Edge>> sub;

    for (int u : verts) {
        for (const auto &e : graph[u]) {
            if (verts.count(e.to) && e.open) {
                sub[u].push_back(e);
            }
        }
    }

    // Prim’s
    int start = *verts.begin();
    unordered_set<int> used;
    used.insert(start);

    using T = pair<int,pair<int,int>>; // (weight, (u,v))
    priority_queue<T, vector<T>, greater<T>> pq;

    for (const auto &e : sub[start]) {
        pq.push({e.time, {start, e.to}});
    }

    int total = 0;

    while (used.size() < verts.size() && !pq.empty()) {
        auto [w, uv] = pq.top(); pq.pop();
        int v = uv.second;
        if (used.count(v)) continue;

        used.insert(v);
        total += w;

        for (const auto &e : sub[v]) {
            if (!used.count(e.to)) pq.push({e.time, {v, e.to}});
        }
    }
    return total;
}

int CampusCompass::shortestPath(int s, int t) {
    auto dist = dijkstra(s);
    if (!dist.count(t) || dist[t] == INT_MAX) return INT_MAX;
    return dist[t];
}

bool CampusCompass::handleVerifySchedule(const string &command) {
    stringstream ss(command);
    string cmd, ufid;
    ss >> cmd;   // "verifySchedule"
    ss >> ufid;

    if (!students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &st = students[ufid];
    if (st.classes.size() <= 1) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Build list of (code, start)
    struct C { string code; int start; };
    vector<C> arr;
    for (const string &c : st.classes) {
        arr.push_back({c, classInfo[c].start});
    }

    // Sort by start time
    sort(arr.begin(), arr.end(), [](auto &a, auto &b){
        return a.start < b.start;
    });

    cout << "Schedule Check for " << st.name << ":" << endl;

    // Check gaps between consecutive classes
    for (int i = 0; i+1 < arr.size(); i++) {
        string c1 = arr[i].code;
        string c2 = arr[i+1].code;

        int loc1 = classInfo[c1].location;
        int loc2 = classInfo[c2].location;

        int travel = shortestPath(loc1, loc2);

        int gap = classInfo[c2].start - classInfo[c1].end;  // minutes

        cout << c1 << " - " << c2 << " ";

        if (travel == INT_MAX) {
            cout << "\"Cannot make it!\"" << endl;
            continue;
        }

        if (travel <= gap)
            cout << "\"Can make it!\"" << endl;
        else
            cout << "\"Cannot make it!\"" << endl;
    }

    return true;
}



bool CampusCompass::ParseCommand(const string &command) {
    stringstream ss(command);
    string cmd;
    ss >> cmd;
    if (cmd == "insert")               return handleInsert(command);
    else if (cmd == "remove")          return handleRemove(command);
    else if (cmd == "dropClass")       return handleDropClass(command);
    else if (cmd == "replaceClass")    return handleReplaceClass(command);
    else if (cmd == "removeClass")     return handleRemoveClassCmd(command);
    else if (cmd == "toggleEdgesClosure") return handleToggleEdgesClosure(command);
    else if (cmd == "checkEdgeStatus") return handleCheckEdgeStatus(command);
    else if (cmd == "isConnected")     return handleIsConnected(command);
    else if (cmd == "printShortestEdges") return handlePrintShortestEdges(command);
    else if (cmd == "printStudentZone")   return handlePrintStudentZone(command);
    else if (cmd == "verifySchedule")  return handleVerifySchedule(command);
    cout << "unsuccessful" << endl;
    return false;
}

static string stripQuotes(const string &s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

bool CampusCompass::handleInsert(const string &command) {
    stringstream ss(command);
    string cmd;
    ss >> cmd; // "insert"

    string nameQuoted;
    ss >> nameQuoted;
    string name = stripQuotes(nameQuoted);

    string ufid;
    ss >> ufid;

    string residenceStr;
    ss >> residenceStr;
    int residence = stoi(residenceStr);

    int n;
    ss >> n;

    // --- Validate N ---
    if (n < 1 || n > 6) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // --- Read exactly N class codes ---
    vector<string> classList;
    for (int i = 0; i < n; i++) {
        string c;
        if (!(ss >> c)) {
            cout << "unsuccessful" << endl;
            return false;
        }
        classList.push_back(c);
    }

    // --- Validate UFID ---
    if (ufid.size() != 8 || !all_of(ufid.begin(), ufid.end(), ::isdigit) || students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // --- Validate name (letters + spaces only) ---
    for (char ch : name) {
        if (!isalpha(ch) && ch != ' ') {
            cout << "unsuccessful" << endl;
            return false;
        }
    }

    // --- Validate class codes ---
    for (const string &c : classList) {
        if (classLocations.count(c) == 0) {
            cout << "unsuccessful" << endl;
            return false;
        }
        if (c.size() != 7 || 
           !isupper(c[0]) || !isupper(c[1]) || !isupper(c[2]) ||
           !isdigit(c[3]) || !isdigit(c[4]) || !isdigit(c[5]) || !isdigit(c[6])) {
            cout << "unsuccessful" << endl;
            return false;
        }
    }

    // --- Insert student ---
    Student s;
    s.name = name;
    s.id = ufid;
    s.residence = residence;
    s.classes = classList;

    students[ufid] = s;

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleRemove(const string &command) {
    stringstream ss(command);
    string cmd, ufid;
    ss >> cmd;      // "remove"
    ss >> ufid;     // STUDENT_ID

    // Validate UFID
    if (ufid.size() != 8 || !all_of(ufid.begin(), ufid.end(), ::isdigit)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Check existence
    if (students.count(ufid) == 0) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Remove student
    students.erase(ufid);

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleDropClass(const string &command) {
    stringstream ss(command);
    string cmd, ufid, classCode;
    ss >> cmd;       // "dropClass"
    ss >> ufid;
    ss >> classCode;

    // Validate UFID
    if (ufid.size() != 8 || !all_of(ufid.begin(), ufid.end(), ::isdigit)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Student must exist
    if (!students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Class must exist globally
    if (!classLocations.count(classCode)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &st = students[ufid];

    // Student must have the class
    auto it = find(st.classes.begin(), st.classes.end(), classCode);
    if (it == st.classes.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Drop class
    st.classes.erase(it);

    // If student now has 0 classes → remove entirely
    if (st.classes.empty()) {
        students.erase(ufid);
    }

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleReplaceClass(const string &command) {
    stringstream ss(command);
    string cmd, ufid, oldClass, newClass;
    ss >> cmd;        // "replaceClass"
    ss >> ufid;
    ss >> oldClass;
    ss >> newClass;

    // Validate UFID
    if (ufid.size() != 8 || !all_of(ufid.begin(), ufid.end(), ::isdigit)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Student must exist
    if (!students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // newClass must exist globally
    if (!classLocations.count(newClass)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &st = students[ufid];

    // Student must have oldClass
    auto itOld = find(st.classes.begin(), st.classes.end(), oldClass);
    if (itOld == st.classes.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Student must NOT already have newClass
    if (find(st.classes.begin(), st.classes.end(), newClass) != st.classes.end()) {
        cout << "unsuccessful" << endl;
        return false;
    }

    // Replace oldClass with newClass
    *itOld = newClass;

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleRemoveClassCmd(const string &command) {
    stringstream ss(command);
    string cmd, classCode;
    ss >> cmd;        // "removeClass"
    ss >> classCode;

    // Class must exist globally
    if (!classLocations.count(classCode)) {
        cout << "0" << endl;  // no students dropped
        return false;
    }

    int droppedCount = 0;
    vector<string> toErase; // UFIDs to remove entirely

    // Iterate over all students
    for (auto &p : students) {
        Student &st = p.second;

        auto it = find(st.classes.begin(), st.classes.end(), classCode);
        if (it != st.classes.end()) {
            droppedCount++;
            st.classes.erase(it);

            // If student now has 0 classes → remove later
            if (st.classes.empty()) {
                toErase.push_back(st.id);
            }
        }
    }

    // Remove students with 0 classes
    for (const string &id : toErase) {
        students.erase(id);
    }

    cout << droppedCount << endl;
    return true;
}

bool CampusCompass::handleToggleEdgesClosure(const string &command) {
    stringstream ss(command);
    string cmd;
    ss >> cmd;   // "toggleEdgesClosure"

    int N;
    ss >> N;

    vector<int> ids;
    ids.reserve(2 * N);

    for (int i = 0; i < 2 * N; i++) {
        int x;
        if (!(ss >> x)) {
            cout << "unsuccessful" << endl;
            return false;
        }
        ids.push_back(x);
    }

    // Process N edges: (ids[0], ids[1]), (ids[2], ids[3]), ...
    for (int i = 0; i < 2 * N; i += 2) {
        int u = ids[i];
        int v = ids[i + 1];

        // toggle u->v
        for (auto &e : graph[u]) {
            if (e.to == v) {
                e.open = !e.open;
                break;
            }
        }
        // toggle v->u
        for (auto &e : graph[v]) {
            if (e.to == u) {
                e.open = !e.open;
                break;
            }
        }
    }

    cout << "successful" << endl;
    return true;
}

bool CampusCompass::handleCheckEdgeStatus(const string &command) {
    stringstream ss(command);
    string cmd;
    int u, v;

    ss >> cmd;  // "checkEdgeStatus"
    ss >> u >> v;

    // Check if u exists in graph
    if (!graph.count(u)) {
        cout << "DNE" << endl;
        return true;
    }

    for (const auto &e : graph[u]) {
        if (e.to == v) {
            cout << (e.open ? "open" : "closed") << endl;
            return true;
        }
    }

    // Edge not found
    cout << "DNE" << endl;
    return true;
}

bool CampusCompass::handleIsConnected(const string &command) {
    stringstream ss(command);
    string cmd;
    int start, goal;

    ss >> cmd;   // "isConnected"
    ss >> start >> goal;

    // If nodes don't exist, cannot be connected
    if (!graph.count(start) || !graph.count(goal)) {
        cout << "unsuccessful" << endl;
        return true;
    }

    // BFS queue + visited set
    unordered_set<int> visited;
    queue<int> q;
    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        int cur = q.front();
        q.pop();

        if (cur == goal) {
            cout << "successful" << endl;
            return true;
        }

        for (const auto &e : graph[cur]) {
            if (e.open && !visited.count(e.to)) {
                visited.insert(e.to);
                q.push(e.to);
            }
        }
    }

    // No path found
    cout << "unsuccessful" << endl;
    return true;
}

bool CampusCompass::handlePrintShortestEdges(const string &command) {
    stringstream ss(command);
    string cmd, ufid;
    ss >> cmd;        // "printShortestEdges"
    ss >> ufid;

    // Student must exist
    if (!students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &st = students[ufid];
    int residence = st.residence;

    // Run Dijkstra from residence
    auto dist = dijkstra(residence);

    // Sort class codes lexographically
    vector<string> sortedClasses = st.classes;
    sort(sortedClasses.begin(), sortedClasses.end());

    // Print name
    cout << "Name: " << st.name << endl;

    // For each class
    for (const string &c : sortedClasses) {
        int loc = classLocations[c];
        int d = dist.count(loc) ? dist[loc] : INT_MAX;

        if (d == INT_MAX)
            cout << c << " | Total Time: -1" << endl;
        else
            cout << c << " | Total Time: " << d << endl;
    }

    return true;
}

bool CampusCompass::handlePrintStudentZone(const string &command) {
    stringstream ss(command);
    string cmd, ufid;
    ss >> cmd;  // "printStudentZone"
    ss >> ufid;

    if (!students.count(ufid)) {
        cout << "unsuccessful" << endl;
        return false;
    }

    Student &st = students[ufid];

    // Dijkstra with parents
    unordered_map<int,int> parent;
    auto dist = dijkstraParent(st.residence, parent);

    // Collect all vertices in all shortest paths
    unordered_set<int> verts;
    verts.insert(st.residence);

    for (const string &c : st.classes) {
        int target = classLocations[c];
        if (dist[target] == INT_MAX) continue;

        int cur = target;
        while (cur != -1) {
            verts.insert(cur);
            cur = parent[cur];
        }
    }

    // Compute MST cost over the subgraph
    int cost = primMST(verts);

    cout << "Student Zone Cost For " << st.name << ": " << cost << endl;
    return true;
}

