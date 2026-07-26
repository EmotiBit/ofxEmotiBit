# EmotiBit SlidePlayer

Displays timed sequences of image slide sets and logs all events to a CSV file. Intended for use in EmotiBit data-collection sessions.

## Settings file

On first launch the app copies `emotibitSlidePlayerSettings.json` to `~/Documents/EmotiBit/EmotiBit SlidePlayer/` and reads it from there on every subsequent run. Edit that copy to configure the app.

### Global slide settings

| Key | Description |
|-----|-------------|
| `background` | Image shown during the off-interval between slides |
| `slideSetIntroSlide` | Intro image shown at the start of every set (unless overridden per-set) |
| `pauseOnSetIntroSlide` | If `true`, app pauses on the intro slide until manually advanced |
| `slideSetIntroSlideTimeMin_msec` / `Max_msec` | Random display time for the intro slide (set min == max for a fixed duration) |
| `slideOrderRandomization` | Randomize slide order within a set |
| `maxSlidesPerSet` | How many slides to show per set |
| `slideOnTimeMin_msec` / `Max_msec` | Random display time per slide (set both to 0 to advance only via key press) |
| `slideOffTimeMin_msec` / `Max_msec` | Random blank-screen time between slides |

### App settings

| Key | Description |
|-----|-------------|
| `startFullScreen` | Start in full-screen mode |
| `startPaused` | If `true`, pause on the first slide when the show starts (one-shot — does not repeat mid-show; resets when the show is restarted with `R`, `S`, or `L`) |
| `logFileDirectory` | Directory for CSV log output (leave empty to use the default: `~/Documents/EmotiBit/EmotiBit SlidePlayer`) |

### Keyboard controls (all keys re-bindable in settings)

| Default key | Action |
|-------------|--------|
| `F` | Toggle full screen |
| `N` | Next slide |
| `B` | Previous slide |
| `P` | Pause / resume |
| `R` | Restart slideshow from beginning |
| `T` | Restart current slide set |
| `S` | Reload settings file |
| `L` | Set log file directory |

### Slide sets

Each entry in `slideSets` points to a directory of images (`.jpg`, `.jpeg`, `.png`, `.bmp`). Per-set settings override the global defaults; unspecified keys inherit from `globalSlideSettings`.

```json
"slideSets": [
  {
    "slideDirectory": "example_slides/set-1",
    "slideOrderRandomization": false,
    "slideSetIntroSlide": "example_slides/set-1/set_1-Intro_slide.png"
  }
]
```

Image paths can be relative (TODO: State the relative path location for windows and mac) or absolute.

## Event log

A CSV file (`dateTime,epochTime(S),event,details`) is written to `logFileDirectory` on each run. `epochTime(S)` is UTC Unix time in seconds with millisecond precision (e.g. `1234567890.123`). To align slide events with physiological signals, correlate this column against the `LocalTimestamp` column in parsed EmotiBit data. Logged events include `SLIDE_ON`, `SLIDE_OFF`, `KEY_RELEASE`, `APP_END`, and others.

> **Note:** If `logFileDirectory` does not exist the log file will silently fail to open. Ensure the directory exists before launching.

## Known TODOs

- `startFullScreen` setting is parsed but not yet implemented.
- Pressing `B` at the start of a set does not cross back to the previous set; it stops at the first slide of the current set.
- Supported image extensions (jpg/jpeg/png/bmp) are hardcoded; other formats are not loaded.
- Time-elapsed tracking on key press only accounts for the ON phase, not the OFF phase.
- Log directory existence is not validated before opening the log file.
