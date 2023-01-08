#include <fstream>
#include <iostream>
//- #include <regex>

#include "../include/ofApp.hpp"
#include "ofMain.h"

//- const std::regex r_config = std::regex(R"(^\d+:(.+))");
//- const std::regex r_pos = std::regex(R"(\((\d+),(\d+)\),)");

int main(int argc, char *argv[])
try {
  // simple arguments check
  if (argc != 3 || !std::ifstream(argv[1]) || !std::ifstream(argv[2])) {
    std::cout << "Please check the arguments, e.g.,\n"
              << "> mapf-visualizer assets/random-32-32-20.map "
                 "assets/demo_random-32-32-20.txt"
              << std::endl;
    return 0;
  }

  // load graph
  ifstream g_ifs(argv[1]);
  Graph g(g_ifs);

  /*
  // // load plan
  auto solution_file = std::ifstream(argv[2]);
  Solution solution;
  std::string line;
  std::smatch m, results;
  while (getline(solution_file, line)) {
    if (std::regex_match(line, results, r_config)) {
      auto s = results[1].str();
      Config c;
      auto iter = s.cbegin();
      while (std::regex_search(iter, s.cend(), m, r_pos)) {
        iter = m[0].second;
        auto x = std::stoi(m[1].str());
        auto y = std::stoi(m[2].str());
        c.push_back(G.U[G.width * y + x]);
      }
      solution.push_back(c);
    }
  }
  solution_file.close();
  */

  agent::Layout layout = {
    { {0, 0}, {1, 4}, {2, 6} },
    { {0, 8}, {1, 9}, {2, 3} },
  };

  agent::Plan plan = {
    {0, {{ {0, 1, 0., 2.}, {1, 5, 2., 2.},      {5, 8, 4., 2*sqrt2}, }} },
    {1, {{ {4, 5, 0., 2.}, {5, 8, 2., 2*sqrt2}, {8, 9, 2+2*sqrt2, 2.}, }} },
    {2, {{ {6, 7, 0., 2.}, {7, 2, 2., 5.},      {2, 3, 7., 1.}, }} },
  };

  // visualize
  ofSetupOpenGL(100, 100, OF_WINDOW);
  ofRunApp(new ofApp(g, layout, plan));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
