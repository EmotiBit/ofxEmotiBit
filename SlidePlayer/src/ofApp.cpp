#include "ofApp.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <random>
#include <string>

#include "json/json.h"
#include "ofAppRunner.h"
#include "ofFileUtils.h"
#include "ofImage.h"
#include "ofUtils.h"

// ── Setup
// ─────────────────────────────────────────────────────────────────────

void ofApp::ensureSettingsFile()
{
    std::string docs_dir =
        ofFilePath::join(ofFilePath::join(ofFilePath::join(ofFilePath::getUserHomeDir(), "Documents"), "EmotiBit"), "SlidePlayer");
    std::string target_path = ofFilePath::join(docs_dir, settings_file_name_);
    if (!ofFile(target_path).exists())
    {
        ofDirectory::createDirectory(docs_dir, false, true);
        ofFile source(ofToDataPath(settings_file_name_), ofFile::Reference, false);
        source.copyTo(target_path, false, false);
    }
    settings_file_name_ = target_path;
}

void ofApp::setup()
{
#ifdef TARGET_OSX
    ofSetDataPathRoot("../Resources");
#endif
    ensureSettingsFile();
    ofSetLogLevel(OF_LOG_SILENT);
    if (!loadAppSettings())
    {
        std::cerr << "App settings file not found. Quitting" << std::endl;
        ofExit();
    }
    startLogToFile();
    logEvent("APP_START", "settings=" + settings_file_name_ +
                              " log_dir=" + app_settings_.log_file_directory_);
    logEvent("SETTINGS_JSON", settings_json_);
    for (int i = 0; i < (int)app_settings_.slide_sets_.size(); i++)
    {
        const auto& ss = app_settings_.slide_sets_[i];
        logEvent("SETTINGS_LOAD",
                 "set_index=" + std::to_string(i) +
                     " dir=" + ss.slide_directory_ + " max_slides=" +
                     std::to_string(ss.settings_.max_slides_per_set_) +
                     " randomize=" +
                     std::to_string(ss.settings_.slide_order_randomization_));
    }
}

bool ofApp::loadAppSettings()
{
    ofFile emotibit_slide_player_settings(settings_file_name_);
    std::string json_str;
    if (emotibit_slide_player_settings.exists())
    {
        ofBuffer buf = emotibit_slide_player_settings.readToBuffer();
        json_str = buf.getText();
    }
    else
    {
        std::cerr << "Error: file not found - "
                  << emotibit_slide_player_settings.getAbsolutePath()
                  << std::endl;
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(json_str, root))
    {
        std::cerr << "JSON parsing failed" << std::endl;
        return false;
    }
    Json::FastWriter writer;
    settings_json_ = writer.write(root);
    // FastWriter appends a trailing newline — strip it for clean CSV embedding
    if (!settings_json_.empty() && settings_json_.back() == '\n')
    {
        settings_json_.pop_back();
    }
    return parseSettings(root);
}

bool ofApp::parseSettings(const Json::Value& settings)
{
    if (settings.isMember("appSettings"))
    {
        const Json::Value& app = settings["appSettings"];

        if (app.isMember("logFileDirectory") &&
            !app["logFileDirectory"].asString().empty())
        {
            app_settings_.log_file_directory_ =
                app["logFileDirectory"].asString();
            // TODO: consider if we want to create the directory if the
            // directory does not exist
        }
        else
        {
            // TODO: Consider if the app should quit if this is not specified
            std::cerr << "Error: log file directory not specified" << std::endl;
        }

        if (app.isMember("startFullScreen") && app["startFullScreen"].isBool())
        {
            app_settings_.start_full_screen_ = app["startFullScreen"].asBool();
        }
        else
        {
            std::cerr << "Error: startFullScreen not specified in settings file"
                      << std::endl;
        }

        if (app.isMember("startPaused") && app["startPaused"].isBool())
        {
            app_settings_.start_paused_ = app["startPaused"].asBool();
        }
        else
        {
            std::cerr << "Error: startPaused not specified in settings file"
                      << std::endl;
        }

        if (app.isMember("keyboardControls"))
        {
            const Json::Value& kc = app["keyboardControls"];
            auto load_key = [](const Json::Value& node, const std::string& key,
                               char& target)
            {
                if (node.isMember(key) && !node[key].asString().empty())
                {
                    target = node[key].asString()[0];
                }
            };
            load_key(kc, "toggleFullScreen",
                     app_settings_.keyboard_controls_.toggle_full_screen_);
            load_key(kc, "nextSlide",
                     app_settings_.keyboard_controls_.next_slide_);
            load_key(kc, "previousSlide",
                     app_settings_.keyboard_controls_.previous_slide_);
            load_key(kc, "pauseSlideshowToggle",
                     app_settings_.keyboard_controls_.pause_slide_show_toggle_);
            load_key(kc, "restartSlideshow",
                     app_settings_.keyboard_controls_.restart_slide_show_);
            load_key(kc, "restartSlideSet",
                     app_settings_.keyboard_controls_.restart_slide_set_);
            load_key(kc, "loadSettingsFile",
                     app_settings_.keyboard_controls_.load_settings_file_);
            load_key(kc, "setLogFileDirectory",
                     app_settings_.keyboard_controls_.set_log_file_directory_);
        }
        else
        {
            std::cerr
                << "Error: keyboardControls not specified in settings file"
                << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: appSettings not specified" << std::endl;
    }

    auto resolve_path = [](const std::string& path) -> std::string
    {
        return ofFilePath::isAbsolute(path) ? path : ofToDataPath(path, true);
    };

    auto load_slide_settings =
        [&resolve_path](const Json::Value& node, AppSettings::SlideSettings& s)
    {
        if (node.isMember("background"))
        {
            s.background_ = resolve_path(node["background"].asString());
        }
        if (node.isMember("slideSetIntroSlide"))
        {
            s.slide_set_intro_slide_ =
                resolve_path(node["slideSetIntroSlide"].asString());
        }
        if (node.isMember("pauseOnSetIntroSlide"))
        {
            s.pause_on_set_intro_slide_ = node["pauseOnSetIntroSlide"].asBool();
        }
        if (node.isMember("slideSetIntroSlideTimeMin_msec"))
        {
            s.slide_set_intro_slide_time_min_msec_ =
                node["slideSetIntroSlideTimeMin_msec"].asFloat();
        }
        if (node.isMember("slideSetIntroSlideTimeMax_msec"))
        {
            s.slide_set_intro_slide_time_max_msec_ =
                node["slideSetIntroSlideTimeMax_msec"].asFloat();
        }
        if (node.isMember("slideOrderRandomization"))
        {
            s.slide_order_randomization_ =
                node["slideOrderRandomization"].asBool();
        }
        if (node.isMember("maxSlidesPerSet"))
        {
            s.max_slides_per_set_ = node["maxSlidesPerSet"].asInt();
        }
        if (node.isMember("slideOnTimeMin_msec"))
        {
            s.slide_on_time_min_msec_ = node["slideOnTimeMin_msec"].asFloat();
        }
        if (node.isMember("slideOnTimeMax_msec"))
        {
            s.slide_on_time_max_msec_ = node["slideOnTimeMax_msec"].asFloat();
        }
        if (node.isMember("slideOffTimeMin_msec"))
        {
            s.slide_off_time_min_msec_ = node["slideOffTimeMin_msec"].asFloat();
        }
        if (node.isMember("slideOffTimeMax_msec"))
        {
            s.slide_off_time_max_msec_ = node["slideOffTimeMax_msec"].asFloat();
        }
    };

    if (settings.isMember("globalSlideSettings"))
    {
        load_slide_settings(settings["globalSlideSettings"],
                            app_settings_.global_slide_settings_);
    }
    else
    {
        std::cerr << "Error: globalSlideSettings not specified" << std::endl;
    }

    if (settings.isMember("slideSets"))
    {
        uint16_t num_slide_sets = settings["slideSets"].size();
        for (int i = 0; i < num_slide_sets; i++)
        {
            AppSettings::SlideSet ss;
            if (settings["slideSets"][i].isMember("slideDirectory"))
            {
                ss.slide_directory_ =
                    resolve_path(settings["slideSets"][i]["slideDirectory"].asString());
                // start from global settings, then apply per-set overrides
                ss.settings_ = app_settings_.global_slide_settings_;
                load_slide_settings(settings["slideSets"][i], ss.settings_);
                app_settings_.slide_sets_.push_back(ss);
            }
            else
            {
                std::cerr << "Warning: slide set " << i
                          << " directory not specified. Skipping" << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "Error: slideSets not specified" << std::endl;
    }
    return true;
}

bool ofApp::startLogToFile()
{
    // TODO: Perform a check on if the log directory exists because it
    // results in a silent fail if the directory does not exist
    std::string log_file_name =
        app_settings_.log_file_directory_ + get_timestamp_() + ".csv";
    event_log_.open(log_file_name);
    if (!event_log_.is_open())
    {
        std::cerr << "Failed to open log file: " << log_file_name << std::endl;
        return false;
    }
    event_log_ << "dateTime,event,details\n";
    log_stream_ = &event_log_;
    return true;
}

void ofApp::logEvent(const std::string& event, const std::string& details)
{
    std::string timestamp = get_timestamp_();
    if (log_stream_ != nullptr)
    {
        *log_stream_ << timestamp << ',' << event << ',' << '"' << details
                     << '"' << '\n';
        log_stream_->flush();
    }
    std::cout << timestamp << ' ' << event << ' ' << details << '\n';
}

// ── Per-frame
// ─────────────────────────────────────────────────────────────────

void ofApp::update()
{
    if (should_exit_)
    {
        ofExit();
        return;
    }
    updateCurrentState();
}

void ofApp::updateCurrentState()
{
    if (CurrentState::SlideState::kSlidePause == current_state_.slide_state_)
    {
        return;
    }
    if (current_state_.init_new_set_)
    {
        current_state_.slide_set_index_++;
        if (current_state_.slide_set_index_ >=
            (int)app_settings_.slide_sets_.size())
        {
            logEvent("APP_END", "reason=end_of_slide_show");
            // TODO: consider if we want an end slide
            should_exit_ = true;
            return;
        }
        current_state_.slide_index_ = -1;
        current_state_.slide_settings_ =
            app_settings_.slide_sets_[current_state_.slide_set_index_]
                .settings_;
        current_state_.slide_dir_ =
            app_settings_.slide_sets_[current_state_.slide_set_index_]
                .slide_directory_;

        current_state_.slide_paths_ =
            load_slide_paths_(current_state_.slide_dir_);

        if (current_state_.slide_paths_.empty())
        {
            logEvent(
                "WARNING",
                "set_index=" + std::to_string(current_state_.slide_set_index_) +
                    " reason=no_slides_found dir=" + current_state_.slide_dir_);
            current_state_.init_new_set_ = true;  // skip to next set
            return;
        }

        // extract intro slide before shuffle/resize so it is not counted
        // towards num_slides and not randomized
        std::string intro_path;
        const std::string& intro =
            current_state_.slide_settings_.slide_set_intro_slide_;
        if (!intro.empty())
        {
            std::string intro_filename = ofFilePath::getFileName(intro);
            auto it = std::find_if(
                current_state_.slide_paths_.begin(),
                current_state_.slide_paths_.end(),
                [&intro_filename](const std::string& path)
                { return ofFilePath::getFileName(path) == intro_filename; });
            if (it != current_state_.slide_paths_.end())
            {
                // intro is inside the set directory — extract it so it is
                // not counted towards max_slides and not randomized
                intro_path = *it;
                current_state_.slide_paths_.erase(it);
            }
            else
            {
                // intro is outside the set directory — use the path directly
                intro_path = intro;
            }
            logEvent(
                "SLIDE_SET_INIT",
                "set_index=" + std::to_string(current_state_.slide_set_index_) +
                    " intro_slide=" + intro_path);
        }

        if (current_state_.slide_settings_.slide_order_randomization_)
        {
            std::shuffle(current_state_.slide_paths_.begin(),
                         current_state_.slide_paths_.end(),
                         std::mt19937{std::random_device{}()});
        }
        int num_slides =
            std::min(current_state_.slide_settings_.max_slides_per_set_,
                     (int)current_state_.slide_paths_.size());
        current_state_.slide_paths_.resize(num_slides);

        if (!intro_path.empty())
        {
            current_state_.slide_paths_.insert(
                current_state_.slide_paths_.begin(), intro_path);
        }

        load_background_image_(current_state_.slide_settings_.background_);
        current_state_.init_new_set_ = false;
        std::string slide_list;
        for (const auto& path : current_state_.slide_paths_)
        {
            slide_list += path + "|";
        }
        logEvent(
            "SLIDE_SET_INIT",
            "set_index=" + std::to_string(current_state_.slide_set_index_) +
                " slide_count=" +
                std::to_string(current_state_.slide_paths_.size()) +
                " slides=" + slide_list);
        // use existing machinery to advance to the next slide
        changeSlide(1);
    }

    // SlideState management
    if (CurrentState::SlideState::kSlideOn == current_state_.slide_state_)
    {
        if (current_state_.state_times_.on_time_ != 0)
        {
            // if on time is set to 0, only key strokes can change
            // slides
            if ((float)get_time_msec_() -
                    (float)current_state_.phase_start_msec_ >
                current_state_.state_times_.on_time_)
            {
                current_state_.slide_state_ =
                    CurrentState::SlideState::kSlideOff;
                current_state_.phase_start_msec_ = get_time_msec_();
                logEvent("SLIDE_OFF",
                         "set_index=" +
                             std::to_string(current_state_.slide_set_index_) +
                             " slide_index=" +
                             std::to_string(current_state_.slide_index_));
            }
        }
    }
    if (CurrentState::SlideState::kSlideOff == current_state_.slide_state_)
    {
        if (current_state_.slide_index_ == 0)
        {
            // increment without wait if this is an intro slide
            // no background for intro slide
            changeSlide(1);
        }
        else
        {
            if ((float)get_time_msec_() -
                    (float)current_state_.phase_start_msec_ >
                current_state_.state_times_.off_time_)
            {
                changeSlide(1);
            }
        }
    }
}

void ofApp::changeSlide(int delta)
{
    current_state_.slide_index_ = current_state_.slide_index_ + delta;
    if (current_state_.slide_index_ < 0)
    {
        // TODO: if we press B enough times, we dont cross over to the previous
        // set, we stay at the beginning of the current set
        current_state_.slide_index_ = 0;
    }
    current_state_.slide_state_ = CurrentState::SlideState::kSlideOn;
    float on_time_min = current_state_.slide_settings_.slide_on_time_min_msec_;
    float on_time_max = current_state_.slide_settings_.slide_on_time_max_msec_;
    float off_time_min =
        current_state_.slide_settings_.slide_off_time_min_msec_;
    float off_time_max =
        current_state_.slide_settings_.slide_off_time_max_msec_;
    if (current_state_.slide_index_ == 0)
    {
        // if first slide, use the intro slide values
        // NOTE: There is no OFF time for intro slides
        on_time_min =
            current_state_.slide_settings_.slide_set_intro_slide_time_min_msec_;
        on_time_max =
            current_state_.slide_settings_.slide_set_intro_slide_time_max_msec_;
    }
    if (on_time_min == on_time_max)
    {
        current_state_.state_times_.on_time_ = on_time_max;
    }
    else
    {
        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist((int)on_time_min,
                                                (int)on_time_max);
        current_state_.state_times_.on_time_ = (float)dist(rng);
    }
    if (off_time_min == off_time_max)
    {
        current_state_.state_times_.off_time_ = off_time_max;
    }
    else
    {
        std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> dist((int)off_time_min,
                                                (int)off_time_max);
        current_state_.state_times_.off_time_ = (float)dist(rng);
    }
    current_state_.phase_start_msec_ = get_time_msec_();
    if (current_state_.slide_index_ >= (int)current_state_.slide_paths_.size())
    {
        current_state_.init_new_set_ = true;
    }
    else
    {
        load_slide_image_(
            current_state_.slide_paths_[current_state_.slide_index_]);
        logEvent(
            "SLIDE_ON",
            "set_index=" + std::to_string(current_state_.slide_set_index_) +
                " slide_index=" + std::to_string(current_state_.slide_index_) +
                " path=" +
                current_state_.slide_paths_[current_state_.slide_index_]);
    }
}

// ── Rendering
// ─────────────────────────────────────────────────────────────────

void ofApp::draw()
{
    if (CurrentState::SlideState::kSlideOff == current_state_.slide_state_)
    {
        drawImageFitted(background_image_);
    }
    else
    {
        drawImageFitted(current_slide_image_);
    }
}

void ofApp::drawImageFitted(const ofImage& img)
{
    if (!img.isAllocated())
    {
        ofBackground(0);
        return;
    }
    float scale = std::min((float)ofGetWidth() / img.getWidth(),
                           (float)ofGetHeight() / img.getHeight());
    float draw_w = img.getWidth() * scale;
    float draw_h = img.getHeight() * scale;
    float x = ((float)ofGetWidth() - draw_w) / 2.0f;
    float y = ((float)ofGetHeight() - draw_h) / 2.0f;
    ofBackground(0);
    img.draw(x, y, draw_w, draw_h);
}

// ── Input
// ─────────────────────────────────────────────────────────────────────

void ofApp::keyPressed(int key)
{
    // TODO: Time since phase change should be calculate for boath ON and OFF
    // Modifier keys (Shift, Ctrl, etc.) produce large keycodes outside the
    // printable ASCII range — skip them to avoid spurious log entries.
    if (key > 127)
    {
        return;
    }
    const char kKeyChar = static_cast<char>(toupper(key));
    logEvent("KEY_PRESS", std::string("key=") + kKeyChar);
    if (app_settings_.keyboard_controls_.next_slide_ == kKeyChar)
    {
        changeSlide(1);
    }
    if (app_settings_.keyboard_controls_.previous_slide_ == kKeyChar)
    {
        changeSlide(-1);
    }
    if (app_settings_.keyboard_controls_.restart_slide_set_ == kKeyChar)
    {
        changeSlide((-1 * current_state_.slide_index_));
    }
    if (app_settings_.keyboard_controls_.restart_slide_show_ == kKeyChar)
    {
        current_state_.slide_set_index_ = -1;
        current_state_.init_new_set_ = true;
    }
    if (app_settings_.keyboard_controls_.pause_slide_show_toggle_ == kKeyChar)
    {
        if (current_state_.slide_state_ !=
            CurrentState::SlideState::kSlidePause)
        {
            current_state_.slide_state_before_pause_ =
                current_state_.slide_state_;
            current_state_.slide_state_ = CurrentState::SlideState::kSlidePause;
            current_state_.time_since_phase_start_on_pause_ =
                (float)get_time_msec_() -
                (float)current_state_.phase_start_msec_;
            logEvent(
                "PAUSE",
                "set_index=" + std::to_string(current_state_.slide_set_index_) +
                    " slide_index=" +
                    std::to_string(current_state_.slide_index_));
        }
        else
        {
            current_state_.slide_state_ =
                current_state_.slide_state_before_pause_;
            current_state_.phase_start_msec_ =
                get_time_msec_() -
                (uint64_t)current_state_.time_since_phase_start_on_pause_;
            logEvent(
                "RESUME",
                "set_index=" + std::to_string(current_state_.slide_set_index_) +
                    " slide_index=" +
                    std::to_string(current_state_.slide_index_));
        }
    }
}

void ofApp::keyReleased(int key)
{
}

void ofApp::mouseMoved(int x, int y)
{
}

void ofApp::mouseDragged(int x, int y, int button)
{
}

void ofApp::mousePressed(int x, int y, int button)
{
}

void ofApp::mouseReleased(int x, int y, int button)
{
}

void ofApp::mouseEntered(int x, int y)
{
}

void ofApp::mouseExited(int x, int y)
{
}

void ofApp::windowResized(int w, int h)
{
}

void ofApp::dragEvent(ofDragInfo dragInfo)
{
}

void ofApp::gotMessage(ofMessage msg)
{
}
