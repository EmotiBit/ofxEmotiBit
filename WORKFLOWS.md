# GitHub Workflows Documentation

This document provides an overview of all GitHub workflows in the ofxEmotiBit repository and explains how they work together to create releases.

## Workflow Inventory

### 1. Build Workflows

#### `build-all-on-macos.yml`
**Trigger:** On every push
**Runner:** Self-hosted macOS
**Purpose:** Builds all three EmotiBit applications on macOS
- Clones repository to OpenFrameworks addons directory
- Builds EmotiBitOscilloscope using Xcode (Release-x86_64 scheme)
- Builds EmotiBitDataParser using Xcode (Release scheme)
- Builds EmotiBitFirmwareInstaller using Xcode (Release scheme)

#### `build-all-on-win.yaml`
**Trigger:** On every push
**Runner:** Self-hosted Windows
**Purpose:** Builds all three EmotiBit applications on Windows
- Clones repository to OpenFrameworks addons directory
- Builds EmotiBitOscilloscope using MSBuild (Release configuration)
- Builds EmotiBitDataParser using MSBuild (Release configuration)
- Builds EmotiBitFirmwareInstaller using MSBuild (Release configuration)

#### `linux-workflows.yaml`
**Trigger:** On every push
**Runner:** Self-hosted Linux
**Purpose:** Orchestrates Linux build and test workflows
- Clones repository to OpenFrameworks addons directory
- Calls reusable workflow to build Oscilloscope
- Calls reusable workflow to build DataParser
- Calls reusable workflow to test DataParser

### 2. Reusable Workflows

#### `reusable-workflow-build-dataParser-linux.yml`
**Trigger:** Called by other workflows
**Runner:** Self-hosted Linux
**Purpose:** Builds EmotiBitDataParser on Linux using make

#### `reusable-workflow-build-oscilloscope-linux.yml`
**Trigger:** Called by other workflows
**Runner:** Self-hosted Linux
**Purpose:** Builds EmotiBitOscilloscope on Linux using make

#### `reusable-workflow-test-dataParser-linux.yml`
**Trigger:** Called by other workflows
**Runner:** Self-hosted Linux
**Purpose:** Runs comprehensive tests on DataParser
- Checks parsed file hash for accuracy
- Finds untested typetags
- Tests parsed output format
- Tests composite payload parsing
- Tests timesync response parsing to RD

### 3. Quality Assurance Workflows

#### `check-installer-project-version.yml`
**Trigger:**
- Pull requests to `dev` branch (opened, reopened, synchronized)
- Manual workflow dispatch
**Runner:** Self-hosted Linux
**Purpose:** Validates that the Installer project version is correct by running a test script

### 4. Artifact Upload Workflows

#### `upload-build-artifact-mac.yml`
**Trigger:** After `build-all-on-macos.yml` completes on `dev` branch
**Runner:** Self-hosted macOS
**Purpose:** Stages and uploads macOS release artifacts
- Creates staging directory
- Moves application binaries (.app bundles) to staging area
- Copies SiLabs CP210x drivers for macOS
- Uploads artifact as `EmotiBitSoftware-macos`

#### `upload-build-artifact-win.yml`
**Trigger:** After `build-all-on-win.yaml` or `check-installer-project-version.yml` completes on `dev` branch
**Runner:** Self-hosted Windows
**Purpose:** Builds Windows installer and uploads Windows artifacts
- Builds EmotiBit installer using Visual Studio devenv
- Creates staging directory
- Copies MSI installer and setup.exe to staging area
- Copies SiLabs CP210x drivers for Windows
- Uploads artifact as `EmotiBitSoftware-Windows`

### 5. Release Workflow

#### `create-draft-release.yml`
**Trigger:**
- When a PR is merged from `dev` to `master` branch
- Manual workflow dispatch
**Runner:** Self-hosted Linux
**Purpose:** Creates a draft GitHub release with all build artifacts
- Downloads macOS artifacts from `upload-build-artifact-mac.yml`
- Downloads Windows artifacts from `upload-build-artifact-win.yml`
- Extracts version number from `src/ofxEmotiBitVersion.h`
- Generates release notes template with installation instructions
- Creates a sample `config.txt` file for WiFi credentials
- Creates a draft release with:
  - Tag: `v{version}`
  - Attached files: macOS zip, Windows zip, config.txt
  - Pre-filled release notes

---

## Release Process Summary

The EmotiBit release process is automated through a series of connected GitHub workflows. Here's how they work together:

### Development Phase (Continuous Integration)

1. **On Every Push to Any Branch:**
   - All three platform-specific build workflows run in parallel:
     - `build-all-on-macos.yml` builds on macOS
     - `build-all-on-win.yaml` builds on Windows
     - `linux-workflows.yaml` builds and tests on Linux
   - This ensures code quality and catches build errors early

2. **On Pull Requests to `dev`:**
   - `check-installer-project-version.yml` validates the installer version
   - Ensures version consistency before merging

### Artifact Staging (dev branch only)

3. **After Successful Builds on `dev` Branch:**
   - `upload-build-artifact-mac.yml` triggers after macOS build completes
     - Packages .app bundles and drivers
     - Uploads as `EmotiBitSoftware-macos` artifact
   - `upload-build-artifact-win.yml` triggers after Windows build completes
     - Builds the Windows installer (.msi)
     - Packages installer and drivers
     - Uploads as `EmotiBitSoftware-Windows` artifact

### Release Creation (master branch)

4. **When `dev` is Merged to `master`:**
   - `create-draft-release.yml` automatically triggers
   - Downloads the latest macOS and Windows artifacts
   - Extracts version from source code (`ofxEmotiBitVersion.h`)
   - Creates a draft release with:
     - Auto-generated tag: `v{version}`
     - Both platform packages attached
     - Template release notes (to be filled in manually)
     - Sample WiFi configuration file

### Manual Steps

5. **Developer Completes Release:**
   - Review the draft release
   - Update release notes with:
     - Feature descriptions
     - Bug fixes
     - Firmware version information
     - PR references
   - Publish the release when ready

### Key Design Features

- **Separation of Concerns:** Build workflows are separate from artifact staging
- **Branch-Specific Behavior:** Artifacts only uploaded from `dev` branch
- **Reusable Components:** Linux workflows use reusable workflow files for better maintainability
- **Draft Releases:** Releases start as drafts, allowing manual review before publishing
- **Version Extraction:** Version automatically extracted from source code to prevent mismatch
- **Platform Coverage:** Automated builds for Windows and macOS; Linux users build from source
- **Driver Inclusion:** Both platform packages include necessary SiLabs USB drivers

### Workflow Dependencies

```
Push to any branch
├── build-all-on-macos.yml
├── build-all-on-win.yaml
└── linux-workflows.yaml
    ├── reusable-workflow-build-oscilloscope-linux.yml
    ├── reusable-workflow-build-dataParser-linux.yml
    └── reusable-workflow-test-dataParser-linux.yml

Push to dev branch (after builds complete)
├── upload-build-artifact-mac.yml
└── upload-build-artifact-win.yml
    └── Builds Windows Installer

PR dev → master (when merged)
└── create-draft-release.yml
    ├── Downloads EmotiBitSoftware-macos artifact
    ├── Downloads EmotiBitSoftware-Windows artifact
    └── Creates draft release with all artifacts
```

### Notes for Maintainers

- All workflows use self-hosted runners requiring environment variables for directory paths
- Windows workflow includes special handling for installer version validation
- Linux builds include comprehensive testing of the DataParser component
- The release process assumes artifacts from the `master` branch workflow runs
- Manual workflow dispatch is available for `create-draft-release.yml` for testing
