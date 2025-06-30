#!/bin/bash

# Ad-hoc Code Signing Script for Local Development
# This allows the app to run locally without Apple Developer certificates

APP_PATH="build_release/TrussAnalysisGUI.app"
ENTITLEMENTS_FILE="entitlements_adhoc.plist"

echo " Ad-hoc signing TrussAnalysisGUI for local development..."

# Check if app exists
if [ ! -d "$APP_PATH" ]; then
    echo "❌ App not found at $APP_PATH"
    echo "Run the build first: ./build_and_package.sh"
    exit 1
fi

# Create entitlements file for ad-hoc signing
cat > "$ENTITLEMENTS_FILE" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
</dict>
</plist>
EOF

# Remove any existing signatures
echo "🧹 Removing existing signatures..."
find "$APP_PATH" -name "_CodeSignature" -exec rm -rf {} + 2>/dev/null || true

# Sign all frameworks with ad-hoc signature
echo " Ad-hoc signing Qt frameworks..."
find "$APP_PATH/Contents/Frameworks" -name "*.framework" -exec codesign --force --sign - {} \; 2>/dev/null || true

# Sign all dylibs with ad-hoc signature
echo " Ad-hoc signing dynamic libraries..."
find "$APP_PATH/Contents/Frameworks" -name "*.dylib" -exec codesign --force --sign - {} \; 2>/dev/null || true

# Sign all Qt plugins
echo " Ad-hoc signing Qt plugins..."
find "$APP_PATH/Contents/PlugIns" -name "*.dylib" -exec codesign --force --sign - {} \; 2>/dev/null || true

# Sign the main executable with ad-hoc signature
echo " Ad-hoc signing main executable..."
codesign --force --sign - --entitlements "$ENTITLEMENTS_FILE" "$APP_PATH/Contents/MacOS/TrussAnalysisGUI"

# Sign the app bundle with ad-hoc signature
echo " Ad-hoc signing app bundle..."
codesign --force --sign - --entitlements "$ENTITLEMENTS_FILE" "$APP_PATH"

# Verify the signature
echo " Verifying ad-hoc signature..."
codesign --verify --verbose=2 "$APP_PATH"

if [ $? -eq 0 ]; then
    echo " Ad-hoc signing completed successfully!"
    echo "🏠 App should now run locally (may require user permission)"
    echo ""
    echo " Next steps:"
    echo "   1. Try running: open $APP_PATH"
    echo "   2. If blocked, go to System Preferences > Security & Privacy"
    echo "   3. Click 'Open Anyway' for TrussAnalysisGUI"
else
    echo "❌ Ad-hoc signing failed!"
    exit 1
fi

# Clean up
rm -f "$ENTITLEMENTS_FILE"
