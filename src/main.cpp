#include <fstream>
#include <iostream>

#include "../include/ofApp.hpp"
#include "ofMain.h"

#include "mapf_r/smt/solver/z3.hpp"

int main(int argc, char *argv[])
try {
  // simple arguments check
  if (argc < 2 || argc > 3) {
    std::cout << "Usage: bin/mapf_r-visualizer <graph> [<layout>], e.g.\n"
              << "bin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l"
              << std::endl;
    return 0;
  }

  // load graph
  ifstream g_ifs(argv[1]);
  expect(g_ifs, "Graph file not readable: "s + argv[1]);
  Graph g(g_ifs);

  ofSetupOpenGL(100, 100, OF_WINDOW);

  if (argc == 2) {
    ofRunApp(new ofApp(g));
    return 0;
  }

  ifstream l_ifs(argv[2]);
  expect(l_ifs, "Layout file not readable: "s + argv[2]);
  agent::Layout layout(l_ifs);

  smt::solver::Z3 solver;
  solver.set_graph(g);
  solver.set_layout(layout);
  agent::Plan plan;
  if (solver.solve(std::cout)) {
    plan = solver.make_plan(std::cout);
  }

  // visualize
  ofRunApp(new ofApp(g, layout, plan));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
