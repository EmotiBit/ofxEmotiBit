# Build and Release Process


## Overview

The CI/CD pipeline has three stages:

1. **Build** — triggered on every push to any branch. Builds all apps on all platforms.
2. **Stage** — triggered on pushes to the `dev` branch only. Packages binaries and uploads them as GitHub Actions artifacts.
3. **Release** — triggered when a PR from `dev` → `master` is merged (or manually). Downloads staged artifacts and creates a draft GitHub release.

```
Push to any branch
├── build-all-on-macos.yml      ──► (dev branch only) stage + upload macOS artifact
├── build-all-on-win.yaml       ──► (dev branch only) build installer + upload Windows artifact
└── linux-workflows.yaml
    ├── reusable-workflow-build-oscilloscope-linux.yml
    ├── reusable-workflow-build-dataParser-linux.yml
    ├── reusable-workflow-test-oscilloscope-linux.yml
    └── reusable-workflow-test-dataParser-linux.yml

PR dev → master (merged)
└── create-draft-release.yml
    ├── Downloads macOS artifact
    ├── Downloads Windows artifact
    └── Creates draft GitHub release
```

**Version source:** All version strings are read from `src/ofxEmotiBitVersion.h`.

**Runners:** All builds run on self-hosted runners. Directory paths are configured via repository variables `ADDONS_DIR` and `OFXEMOTIBIT_DIR`.

---

## Applications in the build

| App | macOS | Windows | Linux |
|-----|-------|---------|-------|
| EmotiBitOscilloscope | Xcode (Release-x86_64) | MSBuild (Release) | make |
| EmotiBitDataParser | Xcode (Release) | MSBuild (Release) | make |
| EmotiBitFirmwareInstaller | Xcode (Release) | MSBuild (Release) | — |
| SlidePlayer | Xcode (Release) | MSBuild (Release) | — |

Linux builds are CI health-checks only — no release artifacts are produced.

---

## Workflow reference

### `build-all-on-macos.yml`
**Trigger:** Every push

1. Clones the repo fresh into the OF addons directory.
2. Builds all four apps in parallel using `xcodebuild`.
3. *(dev branch only)* Stages `.app` bundles and SiLabs VCP drivers into a versioned folder, then uploads the folder as the artifact `EmotiBitSoftware_v{version}-macOS`.

### `build-all-on-win.yaml`
**Trigger:** Every push

1. Clones the repo fresh into the OF addons directory.
2. Builds all four apps in parallel using MSBuild.
3. *(dev branch only)* Installs INNO Setup 6 (downloads if not present), downloads the VC++ Redistributable, and compiles `EmotiBitInstaller.iss` into `EmotiBitInstaller-{version}.exe`.
4. *(dev branch only)* Stages the installer and CP210x drivers, then uploads as `EmotiBitSoftware_v{version}-Windows`.

### `linux-workflows.yaml`
**Trigger:** Every push

Orchestrates four reusable workflows in sequence: build Oscilloscope → build DataParser → test Oscilloscope → test DataParser. No artifacts are uploaded.

### `reusable-workflow-build-oscilloscope-linux.yml` / `reusable-workflow-build-dataParser-linux.yml`
Called by `linux-workflows.yaml`. Runs `make -j` in the respective app directory on the self-hosted Linux runner.

### `reusable-workflow-test-oscilloscope-linux.yml`
Runs the auxiliary control automation test script for the Oscilloscope.

### `reusable-workflow-test-dataParser-linux.yml`
Runs five DataParser test suites: parsed data hash validation, unused typetag detection, output format validation, composite payload parsing, and timesync response parsing.

### `create-draft-release.yml`
**Trigger:** PR from `dev` → `master` merged, or manual dispatch

1. Extracts version from `src/ofxEmotiBitVersion.h`.
2. Downloads the macOS and Windows artifacts from the `dev` branch runs.
3. Generates a release notes template and a sample `config.txt` (WiFi credentials).
4. Creates a **draft** GitHub release tagged `v{version}` with all artifacts attached.

The draft must be reviewed and published manually.

### `validate-version-linux.yml`
**Trigger:** Every push

Checks that the version in `ofxEmotiBitVersion.h` is formatted as `X.Y.Z` and is greater than the version on `dev`. Fails the build if the version is lower.

### `check-installer-project-version.yml`
**Deprecated.** Kept for reference. Previously validated the Visual Studio installer project version; replaced by INNO Setup which reads the version automatically.

### `test-powershell.yml`
**Trigger:** Manual only

Validates that the Windows runner has PowerShell, INNO Setup 6, and the VC++ Redistributable available. Useful for debugging runner environment issues.

---

## Updating the workflows

### Adding a new application

**macOS — `build-all-on-macos.yml`:**
1. Add a new build job (same structure as the existing ones, depending on `clone-macos`).
2. In the `upload-artifact-macos` job, add a `mv` command to move the new `.app` bundle into the staging folder.

**Windows — `build-all-on-win.yaml`:**
1. Add a new build job (same structure as the existing ones, depending on `clone-windows`).
2. Add the new app's output binary to `EmotiBitInstaller.iss` so it is included in the installer.
3. The `build-installer-windows` and `upload-artifact-windows` jobs do not need changes — the installer bundles everything defined in the `.iss` file.

**Linux — `linux-workflows.yaml` + new reusable workflow files:**
1. Create `reusable-workflow-build-{appname}-linux.yml` (copy an existing one, update the directory and make target).
2. Optionally create `reusable-workflow-test-{appname}-linux.yml` if tests exist.
3. Add the corresponding `uses:` call(s) in `linux-workflows.yaml`.

### Updating distributable files (e.g. new drivers, bundled assets)

**macOS artifact (`build-all-on-macos.yml` → `upload-artifact-macos` job):**
Add a `cp -r` command in the staging step to copy the new files into the staging folder before the upload step.

**Windows installer (`EmotiBitInstaller.iss`):**
Add the new files under the `[Files]` section of `EmotiBitInstaller.iss`. The workflow compiles this file as-is, so no workflow change is needed — only the `.iss` file.

**Linux:**
No release artifacts are produced on Linux; no changes needed.
