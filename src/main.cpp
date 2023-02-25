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
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/plan/sample.p"
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l"
         << endl;
    return 0;
  }

  ofSetupOpenGL(100, 100, OF_WINDOW);

  Path path = argv[1];

  Graph g;
  if (argc > 2 || contains({".g"}, path.extension())) {
    // load graph
    ifstream g_ifs(path);
    expect(g_ifs, "Graph file not readable: "s + path.to_string());
    g = {g_ifs};
    assert(!g.cvertices().empty());
  }

  if (argc == 2) {
    // graph only
    if (!g.cvertices().empty()) {
      ofRunApp(new ofApp(g));
      return 0;
    }

    // plan only
    ifstream p_ifs(path);
    expect(p_ifs, "Plan file not readable: "s + path.to_string());
    // only `Global_states`, `Global` requires graph
    ofRunApp(new ofApp(agent::plan::Global_states(p_ifs)));
    return 0;
  }

  assert(!g.cvertices().empty());
  path = argv[2];
  if (contains({".stp", ".sp"}, path.extension())) {
    // load plan
    ifstream st_ifs(path);
    agent::plan::Global_states stplan(st_ifs);
    ofRunApp(new ofApp(g, move(stplan)));
    return 0;
  }

  agent::plan::Global plan;
  if (contains({".p"}, path.extension())) {
    ifstream p_ifs(path);
    expect(p_ifs, "Plan file not readable: "s + path.to_string());
    plan = {p_ifs};
  }
  else {
    ifstream l_ifs(path);
    expect(l_ifs, "Layout file not readable: "s + path.to_string());
    agent::Layout layout(l_ifs);

    smt::solver::Z3 solver;
    solver.set_graph(g);
    solver.set_layout(layout);

    if (solver.solve(cout)) {
      plan = solver.make_plan(cout);
    }
  }

  // visualize
  ofRunApp(new ofApp(g, move(plan)));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
