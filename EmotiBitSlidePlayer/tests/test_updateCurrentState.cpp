#include <catch2/catch_test_macros.hpp>
#include <fstream>
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
    app.get_epoch_msec_ = []() -> uint64_t { return 1000000ULL; };

    // one slide set with the given timing
    ofApp::AppSettings::SlideSet ss;
    ss.slide_directory_ = "./fake/";
    ss.settings_.slide_on_time_min_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_on_time_max_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_set_intro_slide_time_min_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_set_intro_slide_time_max_msec_ = slide_on_time_max_msec;
    ss.settings_.slide_off_time_min_msec_ = slide_off_time_max_msec;
    ss.settings_.slide_off_time_max_msec_ = slide_off_time_max_msec;
    ss.settings_.slide_order_randomization_ = false;
    ss.settings_.max_slides_per_set_ = (int)slide_paths.size();
    app.app_settings_.slide_sets_.push_back(ss);

    return app;
}

static ofApp makeAppWithIntro(std::vector<std::string> slide_paths,
                              uint64_t& fake_time,
                              bool pause_on_intro,
                              float slide_on_time_max_msec = 1000.0f,
                              float slide_off_time_max_msec = 500.0f,
                              std::ostream* log_stream = nullptr)
{
    ofApp app = makeApp(slide_paths, fake_time, slide_on_time_max_msec,
                        slide_off_time_max_msec, log_stream);
    app.app_settings_.slide_sets_[0].settings_.slide_set_intro_slide_ =
        slide_paths[0];
    app.app_settings_.slide_sets_[0].settings_.pause_on_set_intro_slide_ =
        pause_on_intro;
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

TEST_CASE("intro slide transitions to SLIDE_OFF when ON time expires",
          "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f,
                        /*off=*/500.0f);

    app.updateCurrentState();  // init, t=0, index=0, ON

    fake_time = 1500;
    app.updateCurrentState();  // 1500 > 1000 → kSlideOff, index still 0

    REQUIRE(app.current_state_.slide_index_ == 0);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);
}

TEST_CASE("intro slide advances to next slide after off_time expires",
          "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f,
                        /*off=*/500.0f);

    app.updateCurrentState();  // init, t=0, index=0, ON

    fake_time = 1500;
    app.updateCurrentState();  // ON expires → kSlideOff, phase=1500

    fake_time = 2100;
    app.updateCurrentState();  // 2100-1500=600 > 500 → changeSlide(1) → index=1, ON

    REQUIRE(app.current_state_.slide_index_ == 1);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("slide transitions to OFF after max_on_time", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app =
        makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f,
                /*off=*/500.0f);

    app.updateCurrentState();  // init, t=0, index=0, ON

    fake_time = 1500;
    app.updateCurrentState();  // slide 0 ON expires → kSlideOff, phase=1500

    fake_time = 2100;
    app.updateCurrentState();  // slide 0 OFF expires → index=1, ON, phase=2100

    fake_time = 3200;
    app.updateCurrentState();  // 3200-2100=1100 > 1000 → kSlideOff

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);
}

TEST_CASE("slide stays ON before max_on_time elapses", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app =
        makeApp({"intro.jpg", "a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f,
                /*off=*/500.0f);

    app.updateCurrentState();  // init, index=0, ON

    fake_time = 1500;
    app.updateCurrentState();  // slide 0 ON expires → kSlideOff, phase=1500

    fake_time = 2100;
    app.updateCurrentState();  // slide 0 OFF expires → index=1, ON, phase=2100

    fake_time = 2500;
    app.updateCurrentState();  // 2500-2100=400 < 1000 → stays ON

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

    app.updateCurrentState();  // init, t=0, index=0, ON

    fake_time = 1500;
    app.updateCurrentState();  // slide 0 ON expires → kSlideOff, phase=1500

    fake_time = 2100;
    app.updateCurrentState();  // slide 0 OFF expires → index=1, ON, phase=2100

    fake_time = 3200;
    app.updateCurrentState();  // index=1 ON: 3200-2100=1100 > 1000 → OFF, phase=3200

    fake_time = 3800;
    app.updateCurrentState();  // OFF: 3800-3200=600 > 500 → index=2, ON

    REQUIRE(app.current_state_.slide_index_ == 2);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: pause / resume
// ─────────────────────────────────────────────────────

TEST_CASE("pause stops state transitions", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);
    app.load_slide_image_ = [](const std::string&) {};

    app.updateCurrentState();  // init

    // pause
    app.keyReleased('P');
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlidePause);

    // time passes but state must not change
    fake_time = 9999;
    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlidePause);
}

TEST_CASE("resume restores previous state", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, state = ON

    app.keyReleased('P');  // pause
    app.keyReleased('P');  // resume

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: manual slide navigation
// ───────────────────────────────────────

TEST_CASE("next key advances slide index", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg", "c.jpg"}, fake_time);

    app.updateCurrentState();  // init

    app.keyReleased('N');

    REQUIRE(app.current_state_.slide_index_ == 1);
}

TEST_CASE("previous key does not go below 0", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);

    app.updateCurrentState();  // init, slide_index = 0

    app.keyReleased('B');

    REQUIRE(app.current_state_.slide_index_ == 0);
}

// ── Tests: case sensitivity
// ───────────────────────────────────────────────────

TEST_CASE("lowercase pause key does not pause", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);

    app.updateCurrentState();  // init, state = ON

    app.keyReleased('p');  // lowercase — should not match default 'P'

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("lowercase next key does not advance slide", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg", "c.jpg"}, fake_time);

    app.updateCurrentState();  // init, slide_index = 0

    app.keyReleased('n');  // lowercase — should not match default 'N'

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
    app.keyReleased('P');

    REQUIRE(log.str().find("PAUSE") != std::string::npos);
}

TEST_CASE("RESUME event is written to log stream", "[logEvent]")
{
    uint64_t fake_time = 0;
    std::ostringstream log;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, 1000.0f, 500.0f, &log);

    app.updateCurrentState();
    app.keyReleased('P');  // pause
    app.keyReleased('P');  // resume

    REQUIRE(log.str().find("RESUME") != std::string::npos);
}

TEST_CASE("SLIDE_ON event is written to log stream on advance", "[logEvent]")
{
    uint64_t fake_time = 0;
    std::ostringstream log;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, 1000.0f, 500.0f, &log);

    app.updateCurrentState();
    app.keyReleased('N');

    REQUIRE(log.str().find("SLIDE_ON") != std::string::npos);
}

// ── Tests: pause_on_set_intro_slide
// ──────────────────────────────────────────

TEST_CASE("pause_on_set_intro_slide pauses on intro slide", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeAppWithIntro({"intro.jpg", "a.jpg", "b.jpg"}, fake_time,
                                 /*pause_on_intro=*/true);

    app.updateCurrentState();  // init → lands on intro (index 0) → auto-pause

    REQUIRE(app.current_state_.slide_index_ == 0);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlidePause);
}

TEST_CASE("pause_on_set_intro_slide resumes to ON after key press", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeAppWithIntro({"intro.jpg", "a.jpg", "b.jpg"}, fake_time,
                                 /*pause_on_intro=*/true);

    app.updateCurrentState();  // init → auto-pause on intro
    app.keyReleased('P');      // resume

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("pause_on_set_intro_slide=false does not pause on intro slide", "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    ofApp app = makeAppWithIntro({"intro.jpg", "a.jpg", "b.jpg"}, fake_time,
                                 /*pause_on_intro=*/false);

    app.updateCurrentState();  // init → intro shown, no pause

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: load settings file ('S')
// ──────────────────────────────────────────

TEST_CASE("load settings: cancel leaves slide state unchanged", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);
    app.open_file_dialog_ = []() { return std::string(""); };

    app.updateCurrentState();  // init, state = ON

    app.keyReleased('S');

    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("load settings: cancel restores timing so slide does not advance early", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);
    app.open_file_dialog_ = [&fake_time]()
    {
        fake_time += 5000;  // simulate dialog open for 5 seconds
        return std::string("");
    };

    app.updateCurrentState();  // init at t=0, ON

    fake_time = 800;           // 800ms into the 1000ms on-time
    app.keyReleased('S');      // dialog consumes 5s but cancel → timing restored

    fake_time += 100;          // only 100ms more after dialog — should not advance
    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_index_ == 0);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("load settings: valid file updates settings_file_name_", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);
    const std::string kTmpPath = "/tmp/test_settings.json";
    std::ofstream f(kTmpPath);
    f << "{}";
    f.close();
    app.open_file_dialog_ = [&kTmpPath]() { return kTmpPath; };

    app.updateCurrentState();
    app.keyReleased('S');

    REQUIRE(app.settings_file_name_ == kTmpPath);
}

TEST_CASE("load settings: valid file restarts slide show", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time);
    const std::string kTmpPath = "/tmp/test_settings.json";
    std::ofstream f(kTmpPath);
    f << "{}";
    f.close();
    app.open_file_dialog_ = [&kTmpPath]() { return kTmpPath; };

    app.updateCurrentState();
    app.keyReleased('S');

    REQUIRE(app.current_state_.slide_set_index_ == -1);
    REQUIRE(app.current_state_.init_new_set_ == true);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

TEST_CASE("load settings: bad file restores slide state and timing", "[keyReleased]")
{
    uint64_t fake_time = 0;
    ofApp app = makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f);
    app.open_file_dialog_ = []() { return std::string("/nonexistent/bad.json"); };

    app.updateCurrentState();  // init at t=0, ON

    fake_time = 500;
    app.keyReleased('S');      // bad file → timing restored

    fake_time += 300;          // 800ms total into 1000ms on-time — should not advance
    app.updateCurrentState();

    REQUIRE(app.current_state_.slide_index_ == 0);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOn);
}

// ── Tests: end of show
// ────────────────────────────────────────────────────────

TEST_CASE("show ends with background visible and keys disabled",
          "[updateCurrentState]")
{
    uint64_t fake_time = 0;
    // Two real slides (no intro) so we can exhaust the set quickly
    ofApp app =
        makeApp({"a.jpg", "b.jpg"}, fake_time, /*on=*/1000.0f, /*off=*/500.0f);

    app.updateCurrentState();  // init, index=0, ON, phase=0

    fake_time = 1500;
    app.updateCurrentState();  // slide 0 ON expires → kSlideOff, phase=1500

    fake_time = 2100;
    app.updateCurrentState();  // slide 0 OFF expires → index=1, ON, phase=2100

    fake_time = 3200;
    app.updateCurrentState();  // slide 1 ON expires → kSlideOff, phase=3200

    fake_time = 3800;
    app.updateCurrentState();  // slide 1 OFF expires → changeSlide(1) out-of-bounds
                               //   → init_new_set_=true, state still kSlideOff
    app.updateCurrentState();  // init block: past last set → show_ended_=true, kSlideOff

    REQUIRE(app.show_ended_ == true);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);

    // Further updates must not change state
    fake_time = 99999;
    app.updateCurrentState();
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);

    // Key presses must be silently ignored
    app.keyReleased('N');
    app.keyReleased('B');
    app.keyReleased('P');
    REQUIRE(app.show_ended_ == true);
    REQUIRE(app.current_state_.slide_state_ ==
            ofApp::CurrentState::SlideState::kSlideOff);
}
