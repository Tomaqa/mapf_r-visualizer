#include "../include/ofApp.hpp"

#include <fstream>

#include "../include/param.hpp"

#include "mapf_r/agent/plan/alg.hpp"

static pair<double, bool> get_scale(double w, double h)
{
  auto window_max_w = default_screen_width - 2*screen_x_buffer - 2*window_x_buffer;
  auto window_max_h = default_screen_height - window_y_top_buffer - window_y_bottom_buffer;
  auto max_w = window_max_w/w;
  auto max_h = window_max_h/h;
  return {min(max_w, max_h) + 1, max_w < max_h};
}

static void printKeys()
{
  std::cout << "keys for visualizer" << std::endl;
  std::cout << "- p : play or pause" << std::endl;
  std::cout << "- l : loop or not" << std::endl;
  std::cout << "- r : reset" << std::endl;
  std::cout << "- v : show virtual line to goals" << std::endl;
  std::cout << "- f : show agent & vertex id" << std::endl;
  std::cout << "- g : show goals" << std::endl;
  std::cout << "- right : progress" << std::endl;
  std::cout << "- left  : back" << std::endl;
  std::cout << "- up    : speed up" << std::endl;
  std::cout << "- down  : speed down" << std::endl;
  std::cout << "- space : screenshot (saved in Desktop)" << std::endl;
  std::cout << "- c : record to GIF (saved in Desktop)" << std::endl;
  std::cout << "- esc : terminate" << std::endl;
}

ofApp::ofApp(const Graph* gl, graph::Properties g_prop, agent::plan::Global p, agent::plan::Global_states sp)
    : graph_l(gl)
    , graph_prop(move(g_prop))
    , scale_pair(get_scale(width, height))
    , plan(move(p))
    , states_plan(move(sp))
{
  assert(width > 0.);
  assert(height > 0.);
  assert(scale > 0.);
}

ofApp::ofApp(const Graph& g, agent::plan::Global p, agent::plan::Global_states sp)
    : ofApp(&g, graph::make_properties(g), move(p), move(sp))
{ }

ofApp::ofApp(const Graph& g)
    : ofApp(g, agent::plan::Global(), agent::plan::Global_states())
{ }

ofApp::ofApp(const Graph& g, const agent::Layout& l, agent::plan::Global p)
    : ofApp(g, move(p), agent::plan::Global_states())
{
  states_plan = {plan, graph(), l};

  makespan = plan.makespan();

  const int n_agents = plan.size();
  agents.reserve(n_agents);
  for (int i = 0; i < n_agents; ++i) {
    agent::Id aid = i;
    assert(aid == int(agents.size()));
    auto& sid = plan.cstart_id_of(aid);
    auto& start = graph().cvertex(sid);
    agents.emplace_back(aid, l.cradius_of(aid), l.cabs_v_of(aid), start.cpos());
  }

  init();
}

ofApp::ofApp(const Graph* gl, graph::Properties g_prop, agent::plan::Global_states sp)
    : ofApp(gl, move(g_prop), agent::plan::Global(), move(sp))
{
  makespan = states_plan.makespan();

  assert(!states_plan.empty());
  const int n_agents = states_plan.size();
  agents.reserve(n_agents);
  for (int i = 0; i < n_agents; ++i) {
    agent::Id aid = i;
    assert(aid == int(agents.size()));
    auto& states = states_plan.cat(aid);
    const auto radius = states.radius;
    const auto abs_v = states.abs_v;
    auto& first_state = states.front();
    agents.emplace_back(aid, radius, abs_v, first_state.cpos());
  }

  init();
}

ofApp::ofApp(const Graph& g, agent::plan::Global_states sp)
    : ofApp(&g, graph::make_properties(g), move(sp))
{ }

ofApp::ofApp(agent::plan::Global_states sp)
    : ofApp(nullptr, graph::make_properties(sp), sp)
{ }

void ofApp::init()
{
  assert(plan.empty() || plan.size() == agents.size());

  if (states_plan.empty()) return;

  assert(states_plan.size() == agents.size());

  for (auto& [aid, splan] : states_plan) {
    auto& s = splan.back();
    assert(s.cduration() > 0 || s.idle());
    if (s.cduration() == 0) s.get_idle().set_duration(inf);
  }

  std::cout << states_plan << std::endl;

  for (auto& ag : agents) {
    auto& aid = ag.cid();
    auto& states = states_plan.cat(aid);
    auto& st = ag.state();
    st = states.front();
    assert(st == states.front());
    assert(st.dt() == st.cduration());
    assert(st.cduration() > 0);
    first_switch_time_threshold = min<float>(first_switch_time_threshold, st.dt());
  }

  agents_action_idx.resize(agents.size());

  assert(first_switch_time_threshold <= makespan);
  switch_time_threshold = first_switch_time_threshold;
}

template <typename T>
T ofApp::scaled(const T& t) const
{
  return t*scale;
}

Coord ofApp::window_size() const
{
  return {scaled(width-margin) + 2*screen_x_buffer + 2*window_x_buffer,
          scaled(height-margin) + window_y_top_buffer + window_y_bottom_buffer};
}

Coord ofApp::window_min() const
{
  if (!scaled_x) return {scaled(min_x), window_y_bottom_buffer/2};
  return {window_x_buffer/2, scaled(min_y)};
}

Coord ofApp::adjusted_pos(Coord pos) const
{
  pos.y = graph_prop.max.y - pos.y;
  pos = scaled(pos);
  pos += scale/2;
  pos.x += screen_x_buffer + window_x_buffer;
  pos.y += window_y_top_buffer;

  return pos;
}

template <typename T>
Coord ofApp::adjusted_pos_of(const T& t) const
{
  return adjusted_pos(t.cpos());
}

bool ofApp::recording() const
{
  if (!flg_record) return false;
  if (!recording_may_start) return false;
  if (finished) return false;
  return true;
}

void ofApp::setup()
{
  const auto [mx, my] = window_min();
  const auto [w, h] = window_size();
  ofSetWindowShape(w, h);
  ofBackground(Color::bg);
  ofDisableAlphaBlending();
  ofSetCircleResolution(32);
  ofSetFrameRate(30);
  font.load("MuseoModerno-VariableFont_wght.ttf", font_size, true, false, true);

  assert(int(w) == ofGetWidth());
  assert(int(h) == ofGetHeight());

  // setup gui
  gui_panel.setup();
  gui_panel.add(timestep_slider.setup("time step", 0, 0, makespan));
  gui_panel.add(speed_slider.setup("speed", 0.05, 0, 1.));

  cam.setVFlip(true);
  cam.setGlobalPosition(ofVec3f(mx + w/2, my + h/2, 580));
  cam.removeAllInteractions();
  cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);

  // the sizes do not matter too much, it always gets the full view
  // .. but also always with some white borders ..
  record_fbo.allocate(w, h, GL_RGB);
  gif_encoder.setup(w, h, 1./ofGetTargetFrameRate());
  ofAddListener(ofxGifEncoder::OFX_GIF_SAVE_FINISHED, this, &ofApp::onGifSaved);

  printKeys();
}

void ofApp::reset()
{
  flg_record = false;

  timestep_slider = 0;

  finished = false;

  recording_may_start = false;

  gif_encoder.stop();
  // gif_encoder.waitForThread();
  gif_encoder.reset();

  if (states_plan.empty()) return;

  switch_time_threshold = first_switch_time_threshold;
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    ag.state() = states_plan.cat(aid).front();
    agents_action_idx[aid] = 0;
  }
}

template <ofApp::StepMode modeV>
void ofApp::doStep(float step)
{
  static_assert(modeV != StepMode::partial);
  return doStepImpl<modeV>(step, timestep_slider, timestep_slider+step);
}

template <ofApp::StepMode modeV>
void ofApp::doStepImpl(float step, float t, float t_next)
{
  if (states_plan.empty()) return;
  if (finished) return;

  recording_may_start = true;

  assert(t < switch_time_threshold || apx_equal(t, makespan));

  if (t_next < 0) {
    reset();
    return;
  }

  [[maybe_unused]] const bool finish = t_next > makespan;
  if constexpr (modeV == StepMode::partial) {
    assert(!finish);
  }
  else if (finish) {
    assert(t_next >= switch_time_threshold);

    if (flg_loop) {
      reset();
      return;
    }

    doStepAdvanceAgs(makespan - t);
    timestep_slider = makespan;
    onFinish();
    return;
  }

  const bool do_switch = t_next >= switch_time_threshold;
  if (!do_switch) {
    doStepAdvanceAgs(step);
    timestep_slider = t_next;
    return;
  }

  const float partial_step = switch_time_threshold - t;
  const float prev_switch_time_threshold = switch_time_threshold;
  doStepAdvanceAgs(partial_step);
  doStepSwitch(partial_step);
  if (modeV == StepMode::manual || t_next == prev_switch_time_threshold) {
    timestep_slider = prev_switch_time_threshold;
    return;
  }

  step = t_next - prev_switch_time_threshold;
  return doStepImpl<StepMode::partial>(step, prev_switch_time_threshold, t_next);
}

void ofApp::doStepAdvanceAgs(float step)
{
  for (auto& ag : agents) {
    ag.state().advance(step);
  }
}

void ofApp::doStepSwitch(float step)
{
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    auto& st = ag.state();

    auto& curr_action_idx = agents_action_idx[aid];
    const auto& states = states_plan.cat(aid);
    if (curr_action_idx == states.size()) continue;
    assert(st.cend_pos() == states[curr_action_idx].cend_pos());
    assert(st.cduration() == states[curr_action_idx].cduration());

    const auto dt = st.dt();
    if (apx_greater(dt, 0) && float(switch_time_threshold + dt) > switch_time_threshold) continue;

    const auto to_pos = st.cend_pos();
    assert(apx_equal<precision::Low>(st.cpos(), to_pos));

    if (++curr_action_idx == states.size()) {
      ag.set_idle_state(to_pos, inf);
      continue;
    }

    st = states[curr_action_idx];
    assert(st == states[curr_action_idx]);
    assert(apx_equal<precision::Huge>(st.cpos(), to_pos));
    assert(st.cduration() > 0);
  }

  switch_time_threshold = min(agents, [&](auto& ag) -> float {
    auto& aid = ag.cid();
    auto& curr_action_idx = agents_action_idx[aid];
    const auto& states = states_plan.cat(aid);
    if (curr_action_idx == states.size()) return t_inf;
    const float thres = switch_time_threshold + ag.cstate().dt();
    assert(thres > switch_time_threshold);
    return thres;
  });

  if (switch_time_threshold > makespan) {
    assert(apx_equal<precision::Low>(switch_time_threshold, makespan) || switch_time_threshold == t_inf);
    switch_time_threshold = makespan;
  }
}

void ofApp::update()
{
  if (!flg_autoplay) return;

  doStep(speed_slider);
}

static void set_agent_color(const agent::Id& aid)
{
  ofSetColor(Color::agents[aid % Color::agents.size()]);
}

void ofApp::draw()
{
  const auto [w, h] = window_size();
  const auto [mx, my] = window_min();

  if (recording()) {
    record_fbo.begin();
    ofClear(Color::bg);
  }

  cam.begin();

  if (flg_snapshot) {
    ofBeginSaveScreenAsPDF(ofFilePath::getUserHomeDir()
                           + "/Desktop/screenshot-" + ofGetTimestampString()
                           + ".pdf",
                           /*multipage*/ false, /*3D*/ false,
                           // it seems that when starting pos. > 0 only draws border elsewhere
                           // but the actual size of the figure does not change ...
                           ofRectangle(0, 0, w + mx*2, h + my*2)
    );
  }

  // draw edges
  ofSetLineWidth(line_width);
  if (graph_l)
  for (auto& vertex : graph().cvertices()) {
    auto& vid = vertex.cid();
    const Coord pos = adjusted_pos_of(vertex);

    for (auto& nid : vertex.cneighbor_ids()) {
      assert(nid != vid);
      if (vid > nid) continue;
      auto& neighbor = graph().cvertex(nid);
      const Coord npos = adjusted_pos_of(neighbor);
      ofSetColor(Color::edge);
      ofDrawLine(pos.x, pos.y, npos.x, npos.y);
    }
  }

  // draw agents
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    auto& st = ag.cstate();
    const Coord pos = adjusted_pos_of(st);
    set_agent_color(aid);
    ofDrawCircle(pos.x, pos.y, scaled(ag.cradius()));

    /*
    // goal
    if (line_mode == LINE_MODE::STRAIGHT) {
      ofDrawLine(goals[i]->x * scale + window_x_buffer + scale / 2,
                 goals[i]->y * scale + window_y_top_buffer + scale / 2, x, y);
    } else if (line_mode == LINE_MODE::PATH) {
      // next loc
      ofSetLineWidth(2);
      if (t2 <= T) {
        auto u = P->at(t2)[i];
        ofDrawLine(x, y, u->x * scale + window_x_buffer + scale / 2,
                   u->y * scale + window_y_top_buffer + scale / 2);
      }
      for (int t = t1 + 1; t < T; ++t) {
        auto v_from = P->at(t)[i];
        auto v_to = P->at(t + 1)[i];
        if (v_from == v_to) continue;
        ofDrawLine(v_from->x * scale + window_x_buffer + scale / 2,
                   v_from->y * scale + window_y_top_buffer + scale / 2,
                   v_to->x * scale + window_x_buffer + scale / 2,
                   v_to->y * scale + window_y_top_buffer + scale / 2);
      }
      ofSetLineWidth(1);
    }

    // agent at goal
    if (v == goals[i]) {
      ofSetColor(255, 255, 255);
      ofDrawCircle(x, y, agent_rad * 0.7);
    }

    // id
    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(i), x - font_size / 2, y + font_size / 2);
    }
    */
  }

  // draw vertices
  if (graph_l)
  for (auto& vertex : graph().cvertices()) {
    auto& vid = vertex.cid();
    const Coord pos = adjusted_pos_of(vertex);

    //+ support also with states_plan only
    if (const agent::Id* aid_l; flg_goal && (aid_l = plan.find_agent_id_of_goal(vid))) {
      set_agent_color(*aid_l);
    }
    else {
      ofSetColor(Color::vertex);
    }
    ofDrawCircle(pos.x, pos.y, vertex_rad);

    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(vid), pos.x - vertex_rad/2, pos.y - vertex_rad/2 + font_size);
    }
  }

  if (flg_snapshot) {
    ofEndSaveScreenAsPDF();
    flg_snapshot = false;
  }

  cam.end();

  if (!recording() || !gui_panel.isMinimized()) gui_panel.draw();

  if (!recording()) return;

  record_fbo.end();
  assert(record_fbo.getWidth() > 0);
  assert(record_fbo.getHeight() > 0);
  ofSetColor(Color::bg);
  record_fbo.draw(0, 0);

  record_fbo.readToPixels(record_pixels);
  assert(record_pixels.getWidth() > 0);
  assert(record_pixels.getHeight() > 0);
  assert(record_pixels.getBitsPerPixel() == 24);
  gif_encoder.addFrame(record_pixels.getData(),
                       record_pixels.getWidth(),
                       record_pixels.getHeight(),
                       record_pixels.getBitsPerPixel()
  );
}

void ofApp::onFinish()
{
  assert(!finished);
  finished = true;

  if (flg_record) saveRecord();
}

void ofApp::saveRecord()
{
  assert(flg_record);
  flg_record = false;
  recording_may_start = false;

  const string fn = ofFilePath::getUserHomeDir()
                  + "/Desktop/record-" + ofGetTimestampString()
                  + ".gif";
  cout << "saving gif as " << fn << " ..." << endl;
  gif_encoder.save(fn);
}

void ofApp::keyPressed(int key)
{
  switch (key) {
  case 'r':
    return reset();
  case 'p':
    if (!flg_autoplay) {
      flg_autoplay = true;
    }
    else {
      flg_autoplay = false;
      if (!recording()) recording_may_start = false;
    }
    return;
  case 'l':
    flg_loop = !flg_loop;
    return;
  case 'g':
    flg_goal = !flg_goal;
    return;
  case 'f':
    flg_font = !flg_font;
    flg_font &= (scale - font_size > 6);
    return;
  case 'v':
    line_mode = static_cast<LINE_MODE>(((int)line_mode + 1) % (int)LINE_MODE::NUM);
    return;
  case OF_KEY_RIGHT:
    return doStep<StepMode::manual>(speed_slider);
  case OF_KEY_LEFT:
    return doStep<StepMode::manual>(-speed_slider);
  case OF_KEY_UP:
    speed_slider = min<float>(speed_slider + 0.01, speed_slider.getMax());
    return;
  case OF_KEY_DOWN:
    speed_slider = max<float>(speed_slider - 0.01, speed_slider.getMin());
    return;
  case 32:
    flg_snapshot = true;  // space
    return;
  case 'c':
    if (!flg_record) {
      flg_record = true;
    }
    else {
      saveRecord();
    }
    return;
  }
}

void ofApp::keyReleased(int key) {}

void ofApp::mouseMoved(int x, int y) {}

void ofApp::mouseDragged(int x, int y, int button) {}

void ofApp::mousePressed(int x, int y, int button) {}

void ofApp::mouseReleased(int x, int y, int button) {}

void ofApp::mouseEntered(int x, int y) {}

void ofApp::mouseExited(int x, int y) {}

void ofApp::windowResized(int w, int h) {}

void ofApp::gotMessage(ofMessage msg) {}

void ofApp::dragEvent(ofDragInfo dragInfo) {}

void ofApp::onGifSaved(string& fileName)
{
  cout << "saved gif as " << fileName << endl;
  gif_encoder.reset();
}

void ofApp::exit()
{
  ofBaseApp::exit();

  gif_encoder.waitForThread();
}
