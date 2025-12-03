#include "catch/catch_amalgamated.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "../src/CampusCompass.h"

using namespace std;

// Helper: run a list of commands through CampusCompass and capture stdout
static string runCommands(const vector<string>& cmds) {
    CampusCompass c;
    c.ParseCSV("../data/edges.csv", "../data/classes.csv");

    ostringstream oss;
    streambuf* oldBuf = cout.rdbuf(oss.rdbuf());

    for (const auto& cmd : cmds) {
        c.ParseCommand(cmd);
    }

    cout.rdbuf(oldBuf);
    return oss.str();
}

// 1) Basic insert + remove behavior
TEST_CASE("Insert and remove student", "[insert][remove]") {
    vector<string> cmds = {
        R"(insert "Alice" 10000001 1 1 COP3502)",
        R"(remove 10000001)",
        R"(remove 10000001)"
    };

    string output = runCommands(cmds);

    string expected =
        "successful\n"   // insert
        "successful\n"   // first remove
        "unsuccessful\n"; // second remove (already gone)

    REQUIRE(output == expected);
}

// 2) dropClass removes student when last class is dropped
TEST_CASE("dropClass removes student when no classes left", "[dropClass]") {
    vector<string> cmds = {
        R"(insert "Alice" 10000001 1 1 COP3502)",
        R"(dropClass 10000001 COP3502)",
        R"(remove 10000001)"
    };

    string output = runCommands(cmds);

    string expected =
        "successful\n"    // insert
        "successful\n"    // dropClass (student removed internally)
        "unsuccessful\n"; // remove fails because student no longer exists

    REQUIRE(output == expected);
}

// 3) Global removeClass behavior (matches project sample)
TEST_CASE("removeClass sample scenario", "[removeClass]") {
    vector<string> cmds = {
        R"(insert "Brandon" 45679999 20 2 COP3530 MAC2311)",
        R"(insert "Brian" 35459999 21 3 COP3530 CDA3101 MAC2311)",
        R"(insert "Briana" 87879999 22 3 CDA3101 MAC2311 EEL3701)",
        R"(removeClass COP3530)",
        R"(remove 87879999)",
        R"(removeClass CDA3101)",
        R"(printShortestEdges 35459999)"
    };

    string output = runCommands(cmds);

    // From the project statement:
    // After these commands, only Brian remains with MAC2311 reachable in 11 minutes.
    string expected =
        "successful\n"               // insert Brandon
        "successful\n"               // insert Brian
        "successful\n"               // insert Briana
        "2\n"                        // removeClass COP3530 (Brandon & Brian)
        "successful\n"               // remove Briana
        "1\n"                        // removeClass CDA3101 (Brian only)
        "Name: Brian\n"
        "MAC2311 | Total Time: 11\n";

    REQUIRE(output == expected);
}

// 4) toggleEdgesClosure + checkEdgeStatus on a known edge (1-2)
TEST_CASE("toggleEdgesClosure and checkEdgeStatus", "[edges]") {
    // We assume edge (1,2) exists and initially open (per project data).
    vector<string> cmds = {
        R"(checkEdgeStatus 1 2)",
        R"(toggleEdgesClosure 1 1 2)",
        R"(checkEdgeStatus 1 2)",
        R"(toggleEdgesClosure 1 1 2)",
        R"(checkEdgeStatus 1 2)"
    };

    string output = runCommands(cmds);

    string expected =
        "open\n"        // initial state
        "successful\n"  // toggle (close)
        "closed\n"      // now closed
        "successful\n"  // toggle (open)
        "open\n";       // open again

    REQUIRE(output == expected);
}

// 5) printShortestEdges for a single student
TEST_CASE("printShortestEdges for one student", "[shortest]") {
    // Brandon at residence 20 with COP3530 (loc 14) and MAC2311 (loc 18)
    // From the given data, shortest times are:
    // 20 -> 14: 20
    // 20 -> 18: 15
    vector<string> cmds = {
        R"(insert "Brandon" 45679999 20 2 COP3530 MAC2311)",
        R"(printShortestEdges 45679999)"
    };

    string output = runCommands(cmds);

    // Classes are sorted lexicographically: COP3530 then MAC2311
    string expected =
        "successful\n"
        "Name: Brandon\n"
        "COP3530 | Total Time: 20\n"
        "MAC2311 | Total Time: 15\n";

    REQUIRE(output == expected);
}
