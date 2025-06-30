# Code Signing Solutions for 2D Truss Analysis

## Problem Summary

The application was crashing with a **Code Signature Invalid** error when launched from the DMG. This is a common macOS security issue that prevents unsigned applications from running.

### Error Details
- **Exception Type**: `EXC_BAD_ACCESS (SIGKILL (Code Signature Invalid))`
- **Root Cause**: Missing or invalid code signatures on the application and bundled Qt frameworks
- **Impact**: Application unable to launch on macOS due to Gatekeeper security policies

## Solutions Provided

### 1.  Ad-hoc Signing (Local Development)

**Best for**: Development, testing, and local distribution

**Script**: `./sign_adhoc.sh`

**Usage**:
```bash
./sign_adhoc.sh
```

**What it does**:
- Signs the application with ad-hoc signatures
- Signs all Qt frameworks and plugins
- Allows local execution (may require user permission)
- No Apple Developer account required

### 2. 🏢 Developer Signing (Commercial Distribution)

**Best for**: App Store or commercial distribution

**Script**: `./sign_app.sh` (requires modification with your certificate)

**Requirements**:
- Apple Developer account ($99/year)
- Valid Developer ID Application certificate

**Usage**:
```bash
# First, update sign_app.sh with your signing identity
./sign_app.sh
```

### 3.  Automated Build with Signing

**Script**: `./build_and_package.sh`

**What it does**:
- Builds the application
- Bundles Qt dependencies with macdeployqt
- Automatically applies ad-hoc signing
- Creates signed DMG package

**Usage**:
```bash
./build_and_package.sh
```

## Current Status

 **FIXED**: The application now runs successfully with ad-hoc signing
 **TESTED**: GUI launches and computational core works correctly
 **PACKAGED**: DMG includes properly signed application

## Distribution Options

### For Internal/Development Use
1. Use the current ad-hoc signed DMG
2. Users may need to:
   - Right-click → Open (first time)
   - Or go to System Preferences → Security & Privacy → Click "Open Anyway"

### For Public Distribution
1. Obtain Apple Developer account
2. Use `./sign_app.sh` with proper certificate
3. Consider notarization for wider compatibility

## Technical Details

### Code Signing Commands Used
```bash
# Ad-hoc signing (development)
codesign --force --sign - --entitlements entitlements.plist app.app

# Developer signing (distribution)
codesign --force --sign "Developer ID Application: Name (TEAM_ID)" app.app
```

### Files Modified
- `Info.plist.in`: Fixed CFBundleExecutable name
- `CMakeLists.txt`: Added Application.cpp to build
- `Application.cpp`: Fixed logger format strings
- `sign_adhoc.sh`: Ad-hoc signing script
- `sign_app.sh`: Developer signing script
- `build_and_package.sh`: Automated build with signing

## Verification

The application can be verified using:
```bash
codesign --verify --deep --strict --verbose=2 TrussAnalysisGUI.app
```

## Next Steps

1. **For immediate use**: The current ad-hoc signed version works
2. **For distribution**: Consider getting Apple Developer certificate
3. **For automation**: Use the build script for consistent builds

## Additional Resources

- [Apple Code Signing Guide](https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/)
- [macOS Notarization](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [Qt macdeployqt Documentation](https://doc.qt.io/qt-6/macos-deployment.html)
