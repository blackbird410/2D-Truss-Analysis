#!/bin/bash

# Code Signing Script for TrussAnalysisGUI
# Requires: Apple Developer account and valid certificates

APP_PATH="build_release/TrussAnalysisGUI.app"
SIGNING_IDENTITY="Developer ID Application: Your Name (TEAM_ID)"
ENTITLEMENTS_FILE="entitlements.plist"

echo " Code Signing TrussAnalysisGUI..."

# Check if app exists
if [ ! -d "$APP_PATH" ]; then
    echo "❌ App not found at $APP_PATH"
    echo "Run the build first: ./build_and_package.sh"
    exit 1
fi

# Create entitlements file
cat > "$ENTITLEMENTS_FILE" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.app-sandbox</key>
    <false/>
    <key>com.apple.security.cs.allow-jit</key>
    <true/>
    <key>com.apple.security.cs.allow-unsigned-executable-memory</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
</dict>
</plist>
EOF

# Sign all frameworks first
echo " Signing Qt frameworks..."
find "$APP_PATH/Contents/Frameworks" -name "*.framework" -exec codesign --force --verify --verbose --sign "$SIGNING_IDENTITY" {} \;

# Sign all dylibs
echo " Signing dynamic libraries..."
find "$APP_PATH/Contents/Frameworks" -name "*.dylib" -exec codesign --force --verify --verbose --sign "$SIGNING_IDENTITY" {} \;

# Sign the main executable
echo " Signing main executable..."
codesign --force --verify --verbose --sign "$SIGNING_IDENTITY" --entitlements "$ENTITLEMENTS_FILE" "$APP_PATH/Contents/MacOS/TrussAnalysisGUI"

# Sign the app bundle
echo " Signing app bundle..."
codesign --force --verify --verbose --sign "$SIGNING_IDENTITY" --entitlements "$ENTITLEMENTS_FILE" "$APP_PATH"

# Verify the signature
echo " Verifying signature..."
codesign --verify --deep --strict --verbose=2 "$APP_PATH"

if [ $? -eq 0 ]; then
    echo " Code signing completed successfully!"
    echo " App is now ready for distribution"
else
    echo "❌ Code signing failed!"
    exit 1
fi

# Clean up
rm -f "$ENTITLEMENTS_FILE"
