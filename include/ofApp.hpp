#pragma once

#include "mapf_r/graph.hpp"
#include "mapf_r/graph/alg.hpp"
#include "mapf_r/agent/layout.hpp"
#include "mapf_r/agent/plan.hpp"

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxGifEncoder.h"

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
  float first_switch_time_threshold{t_inf};
  float switch_time_threshold{t_inf};
  float _time_threshold{t_inf};
  Vector<Idx> agents_action_idx{};

  bool finished{};

  bool recording_may_start{};

  // flg
  bool flg_autoplay{false};
  bool flg_loop{false};
  bool flg_goal{true};
  bool flg_font{false};
  bool flg_screenshot{false};
  bool flg_record{false};

  enum struct LINE_MODE { STRAIGHT, PATH, NONE, NUM };
  LINE_MODE line_mode{LINE_MODE::STRAIGHT};

  // font
  ofTrueTypeFont font;

  // gui
  ofxFloatSlider timestep_slider;
  ofxFloatSlider speed_slider;
  ofxPanel gui_panel;

  // camera
  ofEasyCam cam;

  // record
  ofxGifEncoder gif_encoder;
  ofFbo record_fbo;
  ofPixels record_pixels;

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

  bool recording() const;
  bool screenshot_or_recording() const;

  enum class StepMode { def = 0, partial, manual };

  void setup() override;
  void reset();
  template <StepMode = {}>
  void doStep(float step);
  template <StepMode = {}>
  void doStepImpl(float step, float t, float t_next);
  void doStepAdvanceAgs(float step);
  void doStepSwitch(float step);
  void update() override;
  void draw() override;

  void onFinish();
  void saveRecord();

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

  void onGifSaved(string& fileName);

  void exit() override;
};
