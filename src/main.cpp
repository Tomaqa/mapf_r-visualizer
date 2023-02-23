#include <fstream>
#include <iostream>

#include "../include/ofApp.hpp"
#include "ofMain.h"

#include "mapf_r/smt/solver/z3.hpp"

#include "tomaqa.hpp"

int main(int argc, char *argv[])
try {
  using namespace tomaqa;
  using namespace std;

  // simple arguments check
  if (argc < 2 || argc > 3) {
    cout << "Usage: bin/mapf_r-visualizer <graph> [<plan> | <layout>], e.g."
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/plan/sample.stp"
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l"
         << endl;
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

  const Path path2 = argv[2];
  if (contains({".stp", ".sp"}, path2.extension())) {
    ifstream st_ifs(path2);
    agent::States_plan stplan(st_ifs);
    ofRunApp(new ofApp(g, move(stplan)));
    return 0;
  }

  ifstream l_ifs(path2);
  expect(l_ifs, "Layout file not readable: "s + argv[2]);
  agent::Layout layout(l_ifs);

  smt::solver::Z3 solver;
  solver.set_graph(g);
  solver.set_layout(layout);
  agent::Plan plan;
  if (solver.solve(cout)) {
    plan = solver.make_plan(cout);
  }

  // visualize
  ofRunApp(new ofApp(g, layout, move(plan)));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
