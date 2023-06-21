#include <fstream>
#include <iostream>

#include "../include/ofApp.hpp"
#include "ofMain.h"

#include "mapf_r/smt/solver/z3.hpp"

#include <tomaqa.hpp>

agent::plan::Global make_plan(bool solve, Graph& g, agent::Layout& layout)
{
  if (solve) {
    smt::solver::Z3 solver;
    solver.set_graph(g);
    solver.set_layout(layout);

    if (solver.solve(cout)) {
      return solver.make_plan(cout);
    }
  }

  return agent::plan::Global(layout);
}

int main(int argc, char *argv[])
try {
  using namespace tomaqa;
  using namespace std;

  bool solve = true;

  if (argc >= 2 && "0"s == argv[argc-1]) {
    solve = false;
    --argc;
  }

  // simple arguments check
  if (argc < 2 || argc > 4) {
    cout << "Usage: bin/mapf_r-visualizer <graph> [<splan> | <layout> [<plan>]], e.g."
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/plan/sample.stp"
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l"
         << "\nbin/mapf_r-visualizer data/graph/sample.g data/layout/sample.l data/layout/sample.p"
         << endl;
    return 0;
  }

  ofSetupOpenGL(100, 100, OF_WINDOW);

  Path path = argv[1];

  Graph g;
  if (argc > 2 || contains({".g", ".mapR"}, path.extension())) {
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

  ifstream l_ifs(path);
  expect(l_ifs, "Layout file not readable: "s + path.to_string());
  agent::Layout layout(l_ifs);

  agent::plan::Global plan;
  if (argc == 4) {
    path = argv[3];
    ifstream p_ifs(path);
    expect(p_ifs, "Plan file not readable: "s + path.to_string());
    plan = {p_ifs};
  }
  else {
    plan = make_plan(solve, g, layout);
  }

  ofRunApp(new ofApp(g, layout, move(plan)));
  return 0;
}
catch (const Error& err) {
  std::cerr << err << std::endl;
  return 1;
}
