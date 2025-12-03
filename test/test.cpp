// #include "catch/catch_amalgamated.hpp"
// #include <iostream>
// #include <sstream>
// #include <vector>
// #include <string>

// #include "../src/CampusCompass.h"

// using namespace std;

// static string runCommands(const vector<string>& cmds) {
//     CampusCompass c;
//     c.ParseCSV("data/edges.csv", "data/classes.csv");

//     ostringstream oss;
//     streambuf* oldBuf = cout.rdbuf(oss.rdbuf());

//     for (const auto& cmd : cmds) {
//         c.ParseCommand(cmd);
//     }

//     cout.rdbuf(oldBuf);
//     return oss.str();
// }

// TEST_CASE("Insert and remove student", "[insert][remove]") {
//     vector<string> cmds = {
//         R"(insert "Alice" 10000001 1 1 COP3502)",
//         R"(remove 10000001)",
//         R"(remove 10000001)"
//     };

//     string output = runCommands(cmds);

//     string expected =
//         "successful\n" 
//         "successful\n"   
//         "unsuccessful\n"; 

//     REQUIRE(output == expected);
// }

// TEST_CASE("dropClass removes student when no classes left", "[dropClass]") {
//     vector<string> cmds = {
//         R"(insert "Alice" 10000001 1 1 COP3502)",
//         R"(dropClass 10000001 COP3502)",
//         R"(remove 10000001)"
//     };

//     string output = runCommands(cmds);

//     string expected =
//         "successful\n"   
//         "successful\n"   
//         "unsuccessful\n"; 

//     REQUIRE(output == expected);
// }

// TEST_CASE("removeClass sample scenario", "[removeClass]") {
//     vector<string> cmds = {
//         R"(insert "Brandon" 45679999 20 2 COP3530 MAC2311)",
//         R"(insert "Brian" 35459999 21 3 COP3530 CDA3101 MAC2311)",
//         R"(insert "Briana" 87879999 22 3 CDA3101 MAC2311 EEL3701)",
//         R"(removeClass COP3530)",
//         R"(remove 87879999)",
//         R"(removeClass CDA3101)",
//         R"(printShortestEdges 35459999)"
//     };

//     string output = runCommands(cmds);


//     string expected =
//         "successful\n"              
//         "successful\n"                  
//         "successful\n"                  
//         "2\n"                           
//         "successful\n"                  
//         "1\n"                       
//         "Name: Brian\n"
//         "MAC2311 | Total Time: 11\n";

//     REQUIRE(output == expected);
// }


// TEST_CASE("toggleEdgesClosure and checkEdgeStatus", "[edges]") {
    
//     vector<string> cmds = {
//         R"(checkEdgeStatus 1 2)",
//         R"(toggleEdgesClosure 1 1 2)",
//         R"(checkEdgeStatus 1 2)",
//         R"(toggleEdgesClosure 1 1 2)",
//         R"(checkEdgeStatus 1 2)"
//     };

//     string output = runCommands(cmds);

//     string expected =
//         "open\n"            
//         "successful\n"  
//         "closed\n"          
//         "successful\n"  
//         "open\n";       

//     REQUIRE(output == expected);
// }


// TEST_CASE("printShortestEdges for one student", "[shortest]") {
   
//     vector<string> cmds = {
//         R"(insert "Brandon" 45679999 20 2 COP3530 MAC2311)",
//         R"(printShortestEdges 45679999)"
//     };

//     string output = runCommands(cmds);

    
//     string expected =
//         "successful\n"
//         "Name: Brandon\n"
//         "COP3530 | Total Time: 20\n"
//         "MAC2311 | Total Time: 15\n";

//     REQUIRE(output == expected);
// }
