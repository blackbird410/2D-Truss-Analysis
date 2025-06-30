#!/bin/bash

echo "Creating code signing certificate directly in Keychain..."

# Remove existing certificate first
security delete-certificate -c "TrussAnalysis Developer" ~/Library/Keychains/login.keychain-db 2>/dev/null || true

# Create a new certificate request and self-sign it using Keychain
# This approach creates both the certificate and private key in keychain
certtool y c k=~/Library/Keychains/login.keychain-db r=/tmp/cert_request.csr

echo "Certificate creation initiated. You may need to approve in Keychain Access."

# Check if certificate was created
sleep 2
echo "Checking for code signing identities:"
security find-identity -v -p codesigning

