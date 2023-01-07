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

ofApp::ofApp(const Graph& g, const agent::Layout& l, const agent::Plan& ps)
    : graph(g)
    , layout(l)
    , plan(ps)
    , scale(get_scale(width, height))
{
  assert(width > 0.);
  assert(height > 0.);
  assert(scale > 0.);

  expect(layout.size() == plan.size(),
         "Size of agent goals and plan mismatch: "s
         + to_string(layout.size()) + " != " + to_string(plan.size()));

  const int n_agents = layout.size();
  agents.reserve(n_agents);
  for (auto& [aid, sid] : layout.cstart_ids()) {
    auto& start = graph.cvertex(sid);
    auto& first_step = plan.cat(aid).csteps().front();
    assert(first_step.from_id == sid);
    auto& next_id = first_step.to_id;
    auto& next = graph.cvertex(next_id);
    agents.emplace_back(aid, 1., 1., make_pair(start.cpos(), next.cpos()));
  }
}

Coord ofApp::adjusted_pos(Coord pos) const
{
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
  gui.add(speed_slider.setup("speed", 0.1, 0, 1));

  cam.setVFlip(true);
  cam.setGlobalPosition(ofVec3f(w/2, h/2 - window_y_top_buffer/2, 580));
  cam.removeAllInteractions();
  cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);

  printKeys();
}

void ofApp::update()
{
  if (!flg_autoplay) return;

  const double step = speed_slider;
  const double t = timestep_slider;
  const double t_next = t + step;

  if (t_next <= makespan) {
    timestep_slider = t_next;

    for (auto& ag : agents) {
      ag.advance(step);
    }
  }
  else if (flg_loop) {
    timestep_slider = 0;

    for (auto& ag : agents) {
      auto& aid = ag.cid();
      auto& first_step = plan.cat(aid).csteps().front();
      ag.set(graph.cvertex(first_step.from_id).cpos(), graph.cvertex(first_step.to_id).cpos());
    }
  }
  else {
    timestep_slider = makespan;
  }
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

  // draw graph
  ofSetLineWidth(10);
  ofFill();
  for (auto& vertex : graph.cvertices()) {
    auto& vid = vertex.cid();
    const Coord pos = adjusted_pos(vertex.cpos());

    for (auto& nid : vertex.cneighbors()) {
      assert(nid != vid);
      if (vid > nid) continue;
      auto& neighbor = graph.cvertex(nid);
      const Coord npos = adjusted_pos(neighbor.cpos());
      ofSetColor(Color::edge);
      ofDrawLine(pos.x, pos.y, npos.x, npos.y);
    }

    if (const agent::Id* aid_l; flg_goal && (aid_l = layout.find_agent_id_of_goal(vid))) {
      set_agent_color(*aid_l);
    }
    else {
      ofSetColor(Color::vertex);
    }
    ofDrawCircle(pos.x, pos.y, vertex_rad);

    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(vid), pos.x + 1, pos.y + font_size + 1);
    }
  }

  // draw agents
  for (auto& ag : agents) {
    auto& aid = ag.cid();
    const Coord pos = adjusted_pos(ag.cpos());
    set_agent_color(aid);
    ofDrawCircle(pos.x, pos.y, agent_rad);

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

  if (flg_snapshot) {
    ofEndSaveScreenAsPDF();
    flg_snapshot = false;
  }

  cam.end();
  gui.draw();
}

void ofApp::keyPressed(int key)
{
  if (key == 'r') timestep_slider = 0;  // reset
  if (key == 'p') flg_autoplay = !flg_autoplay;
  if (key == 'l') flg_loop = !flg_loop;
  if (key == 'g') flg_goal = !flg_goal;
  if (key == 'f') {
    flg_font = !flg_font;
    flg_font &= (scale - font_size > 6);
  }
  if (key == 32) flg_snapshot = true;  // space
  if (key == 'v') {
    line_mode =
        static_cast<LINE_MODE>(((int)line_mode + 1) % (int)LINE_MODE::NUM);
  }
  double t;
  if (key == OF_KEY_RIGHT) {
    t = timestep_slider + speed_slider;
    timestep_slider = std::min(makespan, t);
  }
  if (key == OF_KEY_LEFT) {
    t = timestep_slider - speed_slider;
    timestep_slider = std::max(0., t);
  }
  if (key == OF_KEY_UP) {
    t = speed_slider + 0.001;
    speed_slider = std::min(float(t), speed_slider.getMax());
  }
  if (key == OF_KEY_DOWN) {
    t = speed_slider - 0.001;
    speed_slider = std::max(float(t), speed_slider.getMin());
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
