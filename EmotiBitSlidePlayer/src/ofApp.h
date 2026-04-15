#pragma once

#include <fstream>
#include <functional>

#include "ofMain.h"

namespace Json
{
class Value;
}

/// @brief Main application class for the EmotiBit slide player.
///
/// Manages loading settings, displaying timed sequences of image slide sets,
/// logging events to a CSV file, and handling keyboard input.
class ofApp : public ofBaseApp
{
   public:
    // ── Data types
    // ────────────────────────────────────────────────────────────

    /// @brief All settings loaded from the JSON settings file.
    typedef struct AppSettings
    {
        /// @brief Directory where the CSV log file will be written.
        std::string log_file_directory_ = "";
        // TODO: Need to implement the full screen and start paused
        // implementation
        /// @brief Whether the app starts in full-screen mode.
        bool start_full_screen_ = true;
        /// @brief Whether the slide show starts paused.
        bool start_paused_ = true;
        // TODO: implement load_settings and set_log_directory

        /// @brief Keyboard shortcut bindings.
        typedef struct KeyboardControls
        {
            /// @brief Key to toggle full-screen mode.
            char toggle_full_screen_ = 'F';
            /// @brief Key to advance to the next slide.
            char next_slide_ = 'N';
            /// @brief Key to go back to the previous slide.
            char previous_slide_ = 'B';
            /// @brief Key to toggle pause/resume.
            char pause_slide_show_toggle_ = 'P';
            /// @brief Key to restart the entire slide show from the beginning.
            char restart_slide_show_ = 'R';
            /// @brief Key to restart the current slide set from its first
            /// slide.
            char restart_slide_set_ = 'T';
            /// @brief Key to reload the settings file.
            char load_settings_file_ = 'S';
            /// @brief Key to set the log file directory at runtime.
            char set_log_file_directory_ = 'L';
        } KeyboardControls;

        /// @brief Active keyboard control bindings.
        KeyboardControls keyboard_controls_;

        /// @brief Display and timing settings for a slide set.
        typedef struct SlideSettisgs
        {
            /// @brief Path to the background image shown during slide-off
            /// intervals.
            std::string background_ = "";
            /// @brief Path to the intro slide shown at the start of the set.
            std::string slide_set_intro_slide_ = "";
            /// @brief Whether to pause automatically on the intro slide.
            bool pause_on_set_intro_slide_ = true;
            /// @brief Lower bound (ms) of the random display time for the intro
            /// slide. If equal to max, the value is used directly.
            float slide_set_intro_slide_time_min_msec_ = 0;
            /// @brief Upper bound (ms) of the random display time for the intro
            /// slide. If equal to min, the value is used directly.
            float slide_set_intro_slide_time_max_msec_ = 0;
            /// @brief Whether to randomize slide order within the set.
            bool slide_order_randomization_ = true;
            /// @brief Maximum number of slides to show per set.
            int max_slides_per_set_ = 5;
            /// @brief Lower bound (ms) of the random on-time per slide.
            /// If equal to max, the value is used directly. If both are 0,
            /// only key presses advance the slide.
            float slide_on_time_min_msec_ = 0;
            /// @brief Upper bound (ms) of the random on-time per slide.
            /// If equal to min, the value is used directly. If both are 0,
            /// only key presses advance the slide.
            float slide_on_time_max_msec_ = 0;
            /// @brief Lower bound (ms) of the random off-time between slides.
            /// If equal to max, the value is used directly.
            float slide_off_time_min_msec_ = 0;
            /// @brief Upper bound (ms) of the random off-time between slides.
            /// If equal to min, the value is used directly.
            float slide_off_time_max_msec_ = 0;
        } SlideSettings;

        /// @brief Global slide settings used as defaults for all sets.
        SlideSettisgs global_slide_settings_;

        /// @brief A single slide set — a directory of images and its settings.
        typedef struct SlideSet
        {
            /// @brief Path to the directory containing the slide images.
            std::string slide_directory_ = "";
            /// @brief Per-set settings, inheriting from global with optional
            /// overrides.
            SlideSettings settings_;
        } SlideSet;

        /// @brief Ordered list of slide sets to display during the session.
        std::vector<SlideSet> slide_sets_;
    } AppSettings;

    /// @brief Runtime state of the slide show.
    typedef struct CurrentState
    {
        /// @brief Whether the current set needs to be initialised on the next
        /// update.
        bool init_new_set_ = true;
        /// @brief Index of the active slide set. -1 = uninitialized.
        int slide_set_index_ = -1;
        /// @brief Index of the active slide within the current set.
        int slide_index_ = 0;
        /// @brief Path to the active slide set directory.
        std::string slide_dir_;
        /// @brief Ordered list of image paths for the current set.
        std::vector<std::string> slide_paths_;
        /// @brief Active display/timing settings for the current set.
        AppSettings::SlideSettings slide_settings_;
        /// @brief Timestamp (ms) when the current ON or OFF phase started.
        uint64_t phase_start_msec_ = 0;

        /// @brief Phase states for the slide show state machine.
        enum class SlideState
        {
            kSlideOn,     ///< Slide image is visible.
            kSlideOff,    ///< Screen is blank between slides.
            kSlidePause,  ///< Show is paused by the user.
            kSlideIntro   ///< Intro slide is visible.
        };

        /// @brief Holds the randomly chosen on and off durations for the
        /// current slide, sampled once per slide change from the min/max
        /// ranges in SlideSettings.
        typedef struct StateTimes
        {
            /// @brief Randomly chosen duration (ms) the slide stays visible.
            float on_time_ = 0;
            /// @brief Randomly chosen duration (ms) the screen stays blank
            /// after the slide goes off.
            float off_time_ = 0;
        } StateTimes;
        /// @brief Active on/off durations for the current slide.
        StateTimes state_times_;
        /// @brief Current phase state.
        SlideState slide_state_ = SlideState::kSlideOn;
        /// @brief State that was active before the show was paused.
        SlideState slide_state_before_pause_ = SlideState::kSlideOn;
        /// @brief Elapsed time in the current phase at the moment pause was
        /// triggered (ms).
        float time_since_phase_start_on_pause_ = 0;
    } CurrentState;

    // ── Member variables
    // ──────────────────────────────────────────────────────

    /// @brief Settings loaded from the JSON file.
    AppSettings app_settings_;
    /// @brief Live runtime state of the slide show.
    CurrentState current_state_;
    /// @brief When true, @c update() will call @c ofExit().
    bool should_exit_ = false;
    /// @brief File name of the JSON settings file.
    std::string settings_file_name_ = "emotibitSlidePlayerSettings.json";
    /// @brief Compact single-line JSON snapshot of loaded settings, written to
    /// the log on startup.
    std::string settings_json_;

    /// @brief Returns the current time in milliseconds. Injectable for testing.
    std::function<uint64_t()> get_time_msec_ = []()
    { return static_cast<uint64_t>(ofGetElapsedTimeMillis()); };

    /// @brief Returns a formatted timestamp string. Injectable for testing.
    std::function<std::string()> get_timestamp_ = []()
    { return ofGetTimestampString(); };

    /// @brief Loads and returns sorted image paths from a directory. Injectable
    /// for testing.
    std::function<std::vector<std::string>(const std::string&)>
        load_slide_paths_ = [](const std::string& dir)
    {
        if (!ofDirectory::doesDirectoryExist(dir))
        {
            std::cerr << "Error: slide directory not found: " << dir
                      << std::endl;
            return std::vector<std::string>{};
        }
        ofDirectory d(dir);
        // TODO: consider if other file extensions should be allowed when
        // listing the files
        d.allowExt("jpg");
        d.allowExt("jpeg");
        d.allowExt("png");
        d.allowExt("bmp");
        d.listDir();
        d.sort();
        std::vector<std::string> paths;
        for (int i = 0; i < (int)d.size(); i++)
        {
            paths.push_back(d.getPath(i));
        }
        return paths;
    };

    /// @brief Loads an image into @c current_slide_image_. Injectable for
    /// testing.
    std::function<void(const std::string&)> load_slide_image_ =
        [this](const std::string& path) { current_slide_image_.load(path); };

    /// @brief Loads an image into @c background_image_. Injectable for testing.
    std::function<void(const std::string&)> load_background_image_ =
        [this](const std::string& path) { background_image_.load(path); };

    /// @brief Currently displayed slide image.
    ofImage current_slide_image_;
    /// @brief Background image shown during slide-off intervals.
    ofImage background_image_;
    /// @brief File stream for the CSV event log.
    std::ofstream event_log_;
    /// @brief Pointer to the active log output stream. Points to @c event_log_
    /// after file open. Injectable for testing.
    std::ostream* log_stream_ = nullptr;

    // ── Setup
    // ─────────────────────────────────────────────────────────────────

    /// @brief Ensures the settings file exists in ~/Documents/EmotiBit/.
    /// Copies it from the app bundle Resources if not present, then updates
    /// settings_file_name_ to the absolute path.
    void ensureSettingsFile();

    /// @brief openFrameworks setup callback. Loads settings and opens the log
    /// file.
    void setup();

    /// @brief Reads the JSON settings file from disk and calls parseSettings().
    /// @return True on success, false if the file is missing or unparseable.
    bool loadAppSettings();

    /// @brief Parses a JSON value into @c app_settings_.
    /// @param settings Root JSON object from the settings file.
    /// @return Always returns true; individual missing keys are reported to
    /// stderr.
    bool parseSettings(const Json::Value& settings);

    /// @brief Opens the CSV log file and writes the header row.
    /// @return True if the file was opened successfully.
    bool startLogToFile();

    /// @brief Writes a single event row to the CSV log and echoes it to stdout.
    /// @param event Event name (e.g. "SLIDE_ON", "KEY_PRESS").
    /// @param details Space-separated key=value pairs describing the event.
    void logEvent(const std::string& event, const std::string& details);

    // ── Per-frame
    // ─────────────────────────────────────────────────────────────

    /// @brief openFrameworks per-frame update callback.
    void update();

    /// @brief Drives the slide show state machine: initialises sets, manages
    /// ON/OFF transitions, and advances slides based on timing.
    void updateCurrentState();

    /// @brief Advances or retreats the current slide index by @p delta.
    /// @param delta Positive to go forward, negative to go backward.
    void changeSlide(int delta);

    // ── Rendering
    // ─────────────────────────────────────────────────────────────

    /// @brief openFrameworks per-frame draw callback.
    void draw();

    /// @brief Draws @p img centred and aspect-ratio-fitted to the window.
    /// Fills the window with black if the image is not allocated.
    /// @param img Image to draw.
    void drawImageFitted(const ofImage& img);

    // ── Input
    // ─────────────────────────────────────────────────────────────────

    /// @brief Handles key-down events for slide navigation, pause, and restart.
    /// @param key ASCII key code. Modifier keys (>127) are ignored.
    void keyPressed(int key);

    /// @brief openFrameworks key-release callback (unused).
    void keyReleased(int key);
    /// @brief openFrameworks mouse-move callback (unused).
    void mouseMoved(int x, int y);
    /// @brief openFrameworks mouse-drag callback (unused).
    void mouseDragged(int x, int y, int button);
    /// @brief openFrameworks mouse-press callback (unused).
    void mousePressed(int x, int y, int button);
    /// @brief openFrameworks mouse-release callback (unused).
    void mouseReleased(int x, int y, int button);
    /// @brief openFrameworks mouse-enter callback (unused).
    void mouseEntered(int x, int y);
    /// @brief openFrameworks mouse-exit callback (unused).
    void mouseExited(int x, int y);
    /// @brief openFrameworks window-resize callback (unused).
    void windowResized(int w, int h);
    /// @brief openFrameworks drag-and-drop callback (unused).
    void dragEvent(ofDragInfo dragInfo);
    /// @brief openFrameworks message callback (unused).
    void gotMessage(ofMessage msg);
};
