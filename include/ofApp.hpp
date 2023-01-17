#pragma once

#include "mapf_r/graph.hpp"
#include "mapf_r/graph/alg.hpp"
#include "mapf_r/agent/plan.hpp"

#include "ofMain.h"
#include "ofxGui.h"

using namespace mapf_r;

struct ofApp : ofBaseApp {
  const Graph& graph;
  const graph::Properties graph_prop{graph::make_properties(graph)};
  const agent::Layout& layout;

  const agent::Plan& plan;
  Agents agents{};
  const float makespan = plan.makespan();
  static constexpr float t_inf = limits<float>::infinity();
  float first_time_threshold{t_inf};
  float time_threshold{t_inf};
  Vector<Idx> agents_step_idx{};

  // size
  const double width = graph_prop.width() + 2, height = graph_prop.height() + 2;
  const double scale;
  const double vertex_rad = scale/8;
  const int font_size = max(int(scale/8), 6);

  // flg
  bool flg_autoplay{true};
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

  ofApp(const Graph&, const agent::Layout&, const agent::Plan&);

  Real adjusted_radius_of(const Agent&) const;
  template <typename T>
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
