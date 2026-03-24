#include <catch2/catch_test_macros.hpp>

#include "json/json.h"
#include "ofApp.h"

TEST_CASE("global slide settings are loaded", "[parseSettings]")
{
    Json::Value root;
    root["globalSlideSettings"]["background"] = "./bg.jpg";
    root["globalSlideSettings"]["maxSlidesPerSet"] = 3;
    root["globalSlideSettings"]["slideOrderRandomization"] = false;
    root["globalSlideSettings"]["slideOnTimeMin_msec"] = 1000.0f;
    root["globalSlideSettings"]["slideOnTimeMax_msec"] = 2000.0f;

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.global_slide_settings_.background_ == "./bg.jpg");
    REQUIRE(app.app_settings_.global_slide_settings_.max_slides_per_set_ == 3);
    REQUIRE(
        app.app_settings_.global_slide_settings_.slide_order_randomization_ ==
        false);
    REQUIRE(app.app_settings_.global_slide_settings_.slide_on_time_min_msec_ ==
            1000.0f);
    REQUIRE(app.app_settings_.global_slide_settings_.slide_on_time_max_msec_ ==
            2000.0f);
}

TEST_CASE("per-set settings override global settings", "[parseSettings]")
{
    Json::Value root;
    root["globalSlideSettings"]["maxSlidesPerSet"] = 5;
    root["globalSlideSettings"]["slideOrderRandomization"] = true;
    root["slideSets"][0]["slideDirectory"] = "./set01/";
    root["slideSets"][0]["maxSlidesPerSet"] = 2;  // override

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.slide_sets_[0].settings_.max_slides_per_set_ ==
            2);
    // slideOrderRandomization was not overridden — should inherit global value
    REQUIRE(
        app.app_settings_.slide_sets_[0].settings_.slide_order_randomization_ ==
        true);
}

TEST_CASE("per-set inherits global when not overridden", "[parseSettings]")
{
    Json::Value root;
    root["globalSlideSettings"]["maxSlidesPerSet"] = 5;
    root["slideSets"][0]["slideDirectory"] = "./set01/";

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.slide_sets_[0].settings_.max_slides_per_set_ ==
            5);
}

TEST_CASE("slide set without slideDirectory is skipped", "[parseSettings]")
{
    Json::Value root;
    root["globalSlideSettings"]["maxSlidesPerSet"] = 5;
    root["slideSets"][0]["maxSlidesPerSet"] = 2;  // no slideDirectory

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.slide_sets_.empty());
}

TEST_CASE("app settings are loaded", "[parseSettings]")
{
    Json::Value root;
    root["appSettings"]["startFullScreen"] = false;
    root["appSettings"]["startPaused"] = false;
    root["appSettings"]["logFileDirectory"] = "~/Documents/logs/";

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.start_full_screen_ == false);
    REQUIRE(app.app_settings_.start_paused_ == false);
    REQUIRE(app.app_settings_.log_file_directory_ == "~/Documents/logs/");
}

TEST_CASE("keyboard controls are loaded", "[parseSettings]")
{
    Json::Value root;
    root["appSettings"]["keyboardControls"]["toggleFullScreen"] = "G";
    root["appSettings"]["keyboardControls"]["nextSlide"] = "M";

    ofApp app;
    app.parseSettings(root);

    REQUIRE(app.app_settings_.keyboard_controls_.toggle_full_screen_ == 'G');
    REQUIRE(app.app_settings_.keyboard_controls_.next_slide_ == 'M');
}
