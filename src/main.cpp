#include <fstream>
#include <iostream>

#include "../include/ofApp.hpp"
#include "ofMain.h"

#include "mapf_r/smt/solver/z3.hpp"

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

  ifstream l_ifs(argv[2]);
  agent::Layout layout(l_ifs);

  smt::solver::Z3 solver;
  solver.set_graph(g);
  solver.set_layout(layout);
  agent::Plan plan;
  if (solver.solve(std::cout)) {
    plan = solver.make_plan(std::cout);
  }

  // visualize
  ofSetupOpenGL(100, 100, OF_WINDOW);
  ofRunApp(new ofApp(g, layout, plan));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
