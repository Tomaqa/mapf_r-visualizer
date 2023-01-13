#include "../include/ofApp.hpp"

#include <fstream>

#include "../include/param.hpp"

static double get_scale(double width, double height)
{
  auto window_max_w = default_screen_width - 2*screen_x_buffer - 2*window_x_buffer;
  auto window_max_h = default_screen_height - window_y_top_buffer - window_y_bottom_buffer;
  return std::min(window_max_w/width, window_max_h/height) + 1;
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
  std::cout << "- esc : terminate" << std::endl;
}

ofApp::ofApp(const Graph& g, const agent::Layout& l, const agent::Plan& p)
    : graph(g)
    , layout(l)
    , plan(p)
    , scale(get_scale(width, height))
{
  assert(width > 0.);
  assert(height > 0.);
  assert(scale > 0.);

  expect(!layout.empty(), "No agent goals given!");
  expect(plan.empty() || layout.size() == plan.size(),
         "Size of agent goals and plan mismatch: "s
         + to_string(layout.size()) + " != " + to_string(plan.size()));

  const int n_agents = layout.size();
  agents.reserve(n_agents);
  for (int i = 0; i < n_agents; ++i) {
    agent::Id aid = i;
    assert(aid == int(agents.size()));
    auto& sid = layout.cstart_id_of(aid);
    auto& start = graph.cvertex(sid);
    agents.emplace_back(aid, 0.5, 1., start.cpos());

    if (plan.empty()) continue;

    auto& first_step = plan.cat(aid).cfirst_step();
    assert(first_step.from_id == sid);
    auto& next_id = first_step.to_id;
    auto& next = graph.cvertex(next_id);
    auto& ag = agents.back();
    if (first_step.wait_time == 0) {
      ag.set_v(next.cpos());
      assert(!ag.idle());
      first_time_threshold = min(first_time_threshold, first_step.final_time());
    }
    else {
      ag.reset_v();
      assert(ag.idle());
      first_time_threshold = min(first_time_threshold, first_step.wait_abs_time());
    }
  }

  if (plan.empty()) return;

  agents_step_idx.resize(n_agents);

  assert(first_time_threshold <= makespan);
  time_threshold = first_time_threshold;
}

Real ofApp::adjusted_radius_of(const Agent& ag) const
{
  return ag.cradius()*scale;
}

template <typename T>
Coord ofApp::adjusted_pos_of(const T& t) const
{
  auto pos = t.cpos();
  pos.y = graph_prop.max.y - pos.y;
  pos *= scale;
  pos += scale/2;
  pos.x += window_x_buffer;
  pos.y += window_y_top_buffer;

  return pos;
}

void ofApp::setup()
{
  auto w = width*scale + 2*window_x_buffer;
  auto h = height*scale + window_y_top_buffer + window_y_bottom_buffer;
  ofSetWindowShape(w, h);
  ofBackground(Color::bg);
  ofSetCircleResolution(32);
  ofSetFrameRate(30);
  font.load("MuseoModerno-VariableFont_wght.ttf", font_size, true, false, true);

  // setup gui
  gui.setup();
  gui.add(timestep_slider.setup("time step", 0, 0, makespan));
  gui.add(speed_slider.setup("speed", 0.05, 0, 0.5));

  cam.setVFlip(true);
  cam.setGlobalPosition(ofVec3f(w/2, h/2 - window_y_top_buffer/2, 580));
  cam.removeAllInteractions();
  cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);

  printKeys();
}

void ofApp::reset()
{
  timestep_slider = 0;

  if (plan.empty()) return;

  time_threshold = first_time_threshold;
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    auto& first_step = plan.cat(aid).cfirst_step();
    if (first_step.wait_time == 0) {
      ag.set(graph.cvertex(first_step.from_id).cpos(), graph.cvertex(first_step.to_id).cpos());
      assert(!ag.idle());
    }
    else {
      ag.reset_at_pos(graph.cvertex(first_step.from_id).cpos());
      assert(ag.idle());
    }
    agents_step_idx[aid] = 0;
  }
}

void ofApp::doStep(double step)
{
  if (plan.empty()) return;

  const double t = timestep_slider;
  const double t_next = t + step;

  assert(t < time_threshold || t == makespan);
  assert(!(t_next < time_threshold && t_next > makespan));

  if (t_next < 0) {
    return reset();
  }

  if (t_next > makespan) {
    if (flg_loop) return reset();

    timestep_slider = makespan;
    return;
  }

  if (t_next < time_threshold) {
    timestep_slider = t_next;

    for (auto& ag : agents) {
      ag.advance(step);
    }
    return;
  }

  timestep_slider = time_threshold;
  const double step_to_threshold = time_threshold - t;
  for (auto& ag : agents) {
    const bool idle = ag.idle();
    if (!idle) ag.advance(step_to_threshold);

    auto& aid = ag.cid();
    auto& curr_step_idx = agents_step_idx[aid];
    auto& steps = plan.cat(aid).csteps();
    if (curr_step_idx == steps.size()) continue;
    auto curr_step_l = &steps[curr_step_idx];
    const double ag_time_threshold = idle ? curr_step_l->wait_abs_time() : curr_step_l->final_time();
    if (!apx_equal(time_threshold, ag_time_threshold)) continue;

    auto& to = graph.cvertex(curr_step_l->to_id);
    assert(idle || apx_equal(ag.cpos(), to.cpos(), huge_rel_eps, huge_abs_eps));
    assert(!idle || ag.cpos() == graph.cvertex(curr_step_l->from_id).cpos());

    if (idle) {
      ag.set_v(to.cpos());
      continue;
    }

    if (++curr_step_idx == steps.size()) {
      ag.reset_v();
      continue;
    }

    ++curr_step_l;
    assert(curr_step_l->from_id == to.cid());
    if (curr_step_l->wait_time == 0) {
      auto& new_to = graph.cvertex(curr_step_l->to_id);
      ag.set(to.cpos(), new_to.cpos());
    }
    else {
      ag.reset_at_pos(to.cpos());
    }
  }

  time_threshold = min(agents, [&](auto& ag) -> double {
    auto& aid = ag.cid();
    auto& curr_step_idx = agents_step_idx[aid];
    auto& steps = plan.cat(aid).csteps();
    if (curr_step_idx == steps.size()) return inf;
    auto& curr_step = steps[curr_step_idx];
    return ag.idle() ? curr_step.wait_abs_time() : curr_step.final_time();
  });
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
  cam.begin();
  if (flg_snapshot) {
    ofBeginSaveScreenAsPDF(ofFilePath::getUserHomeDir() +
                               "/Desktop/screenshot-" + ofGetTimestampString() +
                               ".pdf",
                           false);
  }

  ofFill();

  // draw edges
  ofSetLineWidth(10);
  for (auto& vertex : graph.cvertices()) {
    auto& vid = vertex.cid();
    const Coord pos = adjusted_pos_of(vertex);

    for (auto& nid : vertex.cneighbor_ids()) {
      assert(nid != vid);
      if (vid > nid) continue;
      auto& neighbor = graph.cvertex(nid);
      const Coord npos = adjusted_pos_of(neighbor);
      ofSetColor(Color::edge);
      ofDrawLine(pos.x, pos.y, npos.x, npos.y);
    }

    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(vid), pos.x + 1, pos.y + font_size + 1);
    }
  }

  // draw agents
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    const Coord pos = adjusted_pos_of(ag);
    set_agent_color(aid);
    ofDrawCircle(pos.x, pos.y, adjusted_radius_of(ag));

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
  for (auto& vertex : graph.cvertices()) {
    auto& vid = vertex.cid();
    const Coord pos = adjusted_pos_of(vertex);

    if (const agent::Id* aid_l; flg_goal && (aid_l = layout.find_agent_id_of_goal(vid))) {
      set_agent_color(*aid_l);
    }
    else {
      ofSetColor(Color::vertex);
    }
    ofDrawCircle(pos.x, pos.y, vertex_rad);
  }

  if (flg_snapshot) {
    ofEndSaveScreenAsPDF();
    flg_snapshot = false;
  }

  cam.end();
  gui.draw();
}

void ofApp::keyPressed(int key)
{
  switch (key) {
  case 'r':
    return reset();
  case 'p':
    flg_autoplay = !flg_autoplay;
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
  case 32:
    flg_snapshot = true;  // space
    return;
  case 'v':
    line_mode = static_cast<LINE_MODE>(((int)line_mode + 1) % (int)LINE_MODE::NUM);
    return;
  case OF_KEY_RIGHT:
    return doStep(speed_slider);
  case OF_KEY_LEFT:
    return doStep(-speed_slider);
  case OF_KEY_UP:
    speed_slider = std::min(speed_slider + 0.01f, speed_slider.getMax());
    return;
  case OF_KEY_DOWN:
    speed_slider = std::max(speed_slider - 0.01f, speed_slider.getMin());
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
