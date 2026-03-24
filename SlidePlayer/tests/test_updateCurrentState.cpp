#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "ofApp.h"

// ── Helpers
// ───────────────────────────────────────────────────────────────

static ofApp makeApp(std::vector<std::string> slide_paths,
                     uint64_t& fake_time,
                     float slide_on_time_max_msec = 1000.0f,
                     float slide_off_time_max_msec = 500.0f,
                     std::ostream* log_stream = nullptr)
{
    ofApp app;
    app.log_stream_ = log_stream;

    // inject no-op image loaders
    app.load_slide_image_ = [](const std::string&) {};
    app.load_background_image_ = [](const std::string&) {};

    // inject fake path loader
    app.load_slide_paths_ = [slide_paths](const std::string&)
    { return slide_paths; };

    // inject controllable clock and fixed timestamp
    app.get_time_msec_ = [&fake_time]() { return fake_time; };
    app.get_timestamp_ = []() { return std::string("2026-01-01T00:00:00"); };

    // one slide set with the given timing
    ofApp::AppSettings::SlideSet ss;
    ss.slide_directory_ = "./fake/";
    ss.settings_.slide_on_time_max_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_set_intro_slide_time_max_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_off_time_max_msec_ = slide_off_time_max_msec;
    ss.settings_.slide_order_randomization_ = false;
    ss.settings_.max_slides_per_set_ = (int)slide_paths.size();
    app.app_settings_.slide_sets_.push_back(ss);

    return app;
}

// ── Tests: set initialisation
// ─────────────────────────────────────────────────────

TEST_CASE("slide set is initialised on first update", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg", "c.jpg"}, fake_time);

    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_set_index_ == 0);
    REQUIRE(app.current_state_.slide_index_ == 0);
    REQUIRE(app.current_state_.slide_paths_.size() == 3);
    REQUIRE_FALSE(app.current_state_.init_new_set_);
}

TEST_CASE("slide state is ON after set init", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);

    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: ON → OFF transition
// ────────────────────────────────────────────────

TEST_CASE("intro slide advances immediately when ON time expires",
          "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, t=0, index=0 (intro), ON

    fake_time = 1500;
    app.updateCurrentState();  // intro expires → skips OFF → index=1, ON

    REQUIRE(app.current_state_.slide_index_ == 1);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("slide transitions to OFF after max_on_time", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app =
        makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, t=0, index=0 (intro), ON

    fake_time = 1500;
    app.updateCurrentState();  // intro expires → index=1, ON, phase=1500

    fake_time = 3000;
    app.updateCurrentState();  // index=1: 3000-1500=1500 > 1000 → OFF

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);
}

TEST_CASE("slide stays ON before max_on_time elapses", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app =
        makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, index=0 (intro), ON

    fake_time = 1500;
    app.updateCurrentState();  // intro expires → index=1, ON, phase=1500

    fake_time = 2000;
    app.updateCurrentState();  // 2000-1500=500 < 1000 → stays ON

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("slide stays ON forever when max_on_time is 0", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app =
        makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/0.0f);

    app.updateCurrentState();  // init, index=0 (intro), ON — intro time also 0

    fake_time = 99999;
    app.updateCurrentState();  // stays ON forever

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: OFF → next slide
// ───────────────────────────────────────────────────

TEST_CASE("slide advances after max_off_time", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time,
                        /*on=*/1000.0f, /*off=*/500.0f);

    app.updateCurrentState();  // init, t=0, index=0 (intro), ON

    fake_time = 1500;
    app.updateCurrentState();  // intro expires → index=1, ON, phase=1500

    fake_time = 2600;
    app.updateCurrentState();  // index=1 ON: 2600-1500=1100 > 1000 → OFF, phase=2600

    fake_time = 3200;
    app.updateCurrentState();  // OFF: 3200-2600=600 > 500 → index=2, ON

    REQUIRE(app.current_state_.slide_index_ == 2);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: pause / resume
// ─────────────────────────────────────────────────────

TEST_CASE("pause stops state transitions", "[keyPressed]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);
    app.load_slide_image_ = [](const std::string&) {};

    app.updateCurrentState();  // init

    // pause
    app.keyPressed('P');
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlidePause);

    // time passes but state must not change
    fake_time = 9999;
    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlidePause);
}

TEST_CASE("resume restores previous state", "[keyPressed]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, state = ON

    app.keyPressed('P');  // pause
    app.keyPressed('P');  // resume

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: manual slide navigation
// ───────────────────────────────────────

TEST_CASE("next key advances slide index", "[keyPressed]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg", "c.jpg"}, fake_time);

    app.updateCurrentState();  // init

    app.keyPressed('N');

    REQUIRE(app.current_state_.slide_index_ == 1);
}

TEST_CASE("previous key does not go below 0", "[keyPressed]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);

    app.updateCurrentState();  // init, slide_index = 0

    app.keyPressed('B');

    REQUIRE(app.current_state_.slide_index_ == 0);
}

// ── Tests: log output
// ─────────────────────────────────────────────────────────

TEST_CASE("PAUSE event is written to log stream", "[logEvent]")
{
    uint64_t fake_time = 0;
    std::ostringstream log;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, 1000.0f, 500.0f, &log);

    app.updateCurrentState();
    app.keyPressed('P');

    REQUIRE(log.str().find("PAUSE") != std::string::npos);
}

TEST_CASE("RESUME event is written to log stream", "[logEvent]")
{
    uint64_t fake_time = 0;
    std::ostringstream log;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, 1000.0f, 500.0f, &log);

    app.updateCurrentState();
    app.keyPressed('P');  // pause
    app.keyPressed('P');  // resume

    REQUIRE(log.str().find("RESUME") != std::string::npos);
}

TEST_CASE("SLIDE_ON event is written to log stream on advance", "[logEvent]")
{
    uint64_t fake_time = 0;
    std::ostringstream log;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, 1000.0f, 500.0f, &log);

    app.updateCurrentState();
    app.keyPressed('N');

    REQUIRE(log.str().find("SLIDE_ON") != std::string::npos);
}
