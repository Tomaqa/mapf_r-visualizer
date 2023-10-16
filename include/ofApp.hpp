#pragma once

#include "mapf_r/graph.hpp"
#include "mapf_r/graph/alg.hpp"
#include "mapf_r/agent/layout.hpp"
#include "mapf_r/agent/plan.hpp"

#include "ofMain.h"
#include "ofxGui.h"

using namespace mapf_r;

struct ofApp : ofBaseApp {
  const Graph* graph_l;
  const Graph& graph() const { assert(graph_l); return *graph_l; }
  graph::Properties graph_prop;

  // size
  static constexpr double margin = 1;
  const double width = graph_prop.width() + 2*margin, height = graph_prop.height() + 2*margin;
  const double min_x = graph_prop.min.x, min_y = graph_prop.min.y;
  const pair<double, bool> scale_pair;
  const double scale = scale_pair.first;
  const bool scaled_x = scale_pair.second;
  const double vertex_rad = scale/8;
  const double line_width = vertex_rad/2;
  const int font_size = max(int(scale/8), 6);

  const agent::plan::Global plan;
  agent::plan::Global_states states_plan;
  Agents agents{};
  float makespan{};
  static constexpr float t_inf = limits<float>::infinity();
  float first_time_threshold{t_inf};
  float time_threshold{t_inf};
  Vector<Idx> agents_action_idx{};

  // flg
  bool flg_autoplay{false};
  bool flg_loop{false};
  bool flg_goal{true};
  bool flg_font{false};
  bool flg_snapshot{false};

  enum struct LINE_MODE { STRAIGHT, PATH, NONE, NUM };
  LINE_MODE line_mode{LINE_MODE::STRAIGHT};

  // font
  ofTrueTypeFont font;

  // gui
  ofxFloatSlider timestep_slider;
  ofxFloatSlider speed_slider;
  ofxPanel gui;

  // camera
  ofEasyCam cam;

  ofApp(const Graph*, graph::Properties, agent::plan::Global, agent::plan::Global_states);
  ofApp(const Graph&, agent::plan::Global, agent::plan::Global_states);
  ofApp(const Graph&);
  ofApp(const Graph&, const agent::Layout&, agent::plan::Global);
  ofApp(const Graph*, graph::Properties, agent::plan::Global_states);
  ofApp(const Graph&, agent::plan::Global_states);
  ofApp(agent::plan::Global_states);

  void init();

  template <typename  T>
  T scaled(const T&) const;
  Coord window_size() const;
  Coord window_min() const;
  Coord adjusted_pos(Coord) const;
  template <typename  T>
  Coord adjusted_pos_of(const T&) const;

  void setup() override;
  void reset();
  void doStep(float step);
  void update() override;
  void draw() override;

  void keyPressed(int key) override;
  void keyReleased(int key) override;
  void mouseMoved(int x, int y) override;
  void mouseDragged(int x, int y, int button) override;
  void mousePressed(int x, int y, int button) override;
  void mouseReleased(int x, int y, int button) override;
  void mouseEntered(int x, int y) override;
  void mouseExited(int x, int y) override;
  void windowResized(int w, int h) override;
  void dragEvent(ofDragInfo dragInfo) override;
  void gotMessage(ofMessage msg) override;
};
