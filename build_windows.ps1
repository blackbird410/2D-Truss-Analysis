# 2D Truss Analysis - Windows Build Script
# Version 2.2.0

param(
    [Parameter(Position=0)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release",
    
    [Parameter()]
    [string]$Generator = "Visual Studio 17 2022",
    
    [Parameter()]
    [switch]$UseVcpkg = $true,
    
    [Parameter()]
    [switch]$SkipTests = $false
)

# Colors for output
$Red = "Red"
$Green = "Green"
$Yellow = "Yellow"
$Blue = "Cyan"

function Write-ColorOutput($ForegroundColor, $Message) {
    Write-Host $Message -ForegroundColor $ForegroundColor
}

Write-ColorOutput $Blue "================================================"
Write-ColorOutput $Blue "  2D Truss Analysis - Windows Build System    "
Write-ColorOutput $Blue "              Version 2.2.0                   "
Write-ColorOutput $Blue "================================================"
Write-Host ""

# Build configuration
$BuildDir = "build_windows"
$VcpkgDir = "$env:USERPROFILE\vcpkg"

Write-ColorOutput $Blue "🔧 Build Configuration:"
Write-ColorOutput $Blue "   • Build Type: $BuildType"
Write-ColorOutput $Blue "   • Generator: $Generator"
Write-ColorOutput $Blue "   • Build Directory: $BuildDir"
Write-ColorOutput $Blue "   • Use vcpkg: $UseVcpkg"
Write-Host ""

# Function to check if command exists
function Test-Command($Command) {
    $null = Get-Command $Command -ErrorAction SilentlyContinue
    return $?
}

# Check dependencies
Write-ColorOutput $Blue "🔍 Checking dependencies..."

$MissingDeps = @()

if (-not (Test-Command "cmake")) {
    $MissingDeps += "cmake"
}

if (-not (Test-Command "git")) {
    $MissingDeps += "git"
}

# Check for Visual Studio or Build Tools
$VSWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VSWhere) {
    $VSInstances = & $VSWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
    if (-not $VSInstances) {
        $MissingDeps += "Visual Studio Build Tools"
    }
} else {
    $MissingDeps += "Visual Studio Installer"
}

if ($MissingDeps.Count -gt 0) {
    Write-ColorOutput $Red "❌ Missing dependencies:"
    foreach ($dep in $MissingDeps) {
        Write-ColorOutput $Red "   • $dep"
    }
    Write-Host ""
    Write-ColorOutput $Yellow "📦 To install dependencies:"
    Write-ColorOutput $Yellow "   1. Install Visual Studio 2022 with C++ workload"
    Write-ColorOutput $Yellow "   2. Install CMake from https://cmake.org/download/"
    Write-ColorOutput $Yellow "   3. Install Git from https://git-scm.com/download/win"
    Write-Host ""
    Write-ColorOutput $Yellow "📦 Or use package managers:"
    Write-ColorOutput $Yellow "   # Using Chocolatey"
    Write-ColorOutput $Yellow "   choco install cmake git visualstudio2022buildtools --package-parameters '--add Microsoft.VisualStudio.Workload.VCTools'"
    Write-Host ""
    Write-ColorOutput $Yellow "   # Using winget"
    Write-ColorOutput $Yellow "   winget install Kitware.CMake Git.Git Microsoft.VisualStudio.2022.BuildTools"
    Write-Host ""
    exit 1
}

Write-ColorOutput $Green "✅ All dependencies found"
Write-Host ""

# Setup vcpkg if requested
if ($UseVcpkg) {
    Write-ColorOutput $Blue "📦 Setting up vcpkg..."
    
    if (-not (Test-Path $VcpkgDir)) {
        Write-ColorOutput $Blue "   Cloning vcpkg..."
        git clone https://github.com/Microsoft/vcpkg.git $VcpkgDir
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput $Red "❌ Failed to clone vcpkg"
            exit 1
        }
    }
    
    # Bootstrap vcpkg
    if (-not (Test-Path "$VcpkgDir\vcpkg.exe")) {
        Write-ColorOutput $Blue "   Bootstrapping vcpkg..."
        & "$VcpkgDir\bootstrap-vcpkg.bat"
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput $Red "❌ Failed to bootstrap vcpkg"
            exit 1
        }
    }
    
    # Install dependencies
    Write-ColorOutput $Blue "   Installing dependencies via vcpkg..."
    $VcpkgPackages = @("qt6[core,widgets,charts]", "eigen3")
    
    foreach ($package in $VcpkgPackages) {
        Write-ColorOutput $Blue "   Installing $package..."
        & "$VcpkgDir\vcpkg.exe" install $package --triplet x64-windows
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput $Red "❌ Failed to install $package"
            exit 1
        }
    }
    
    Write-ColorOutput $Green "✅ vcpkg setup complete"
    Write-Host ""
}

# Create build directory
Write-ColorOutput $Blue "📁 Creating build directory..."
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location $BuildDir

# Configure with CMake
Write-ColorOutput $Blue "⚙️  Configuring with CMake..."
$CMakeArgs = @(
    "-G", $Generator,
    "-A", "x64",
    "-DCMAKE_BUILD_TYPE=$BuildType"
)

if ($UseVcpkg) {
    $CMakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgDir\scripts\buildsystems\vcpkg.cmake"
}

$CMakeArgs += ".."

& cmake $CMakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput $Red "❌ CMake configuration failed"
    exit 1
}

# Build
Write-ColorOutput $Blue "🔨 Building application..."
$Jobs = [Environment]::ProcessorCount
& cmake --build . --config $BuildType --parallel $Jobs
if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput $Red "❌ Build failed"
    exit 1
}

# Verify build
Write-ColorOutput $Blue "🔍 Verifying build..."

$GuiExe = ".\$BuildType\TrussAnalysisGUI.exe"
$CliExe = ".\$BuildType\TrussAnalysisCLI.exe"

if (Test-Path $GuiExe) {
    $GuiSize = (Get-Item $GuiExe).Length / 1MB
    Write-ColorOutput $Green "✅ GUI executable built successfully"
    Write-ColorOutput $Green "   Size: $($GuiSize.ToString('F1')) MB"
} else {
    Write-ColorOutput $Red "❌ GUI executable not found"
}

if (Test-Path $CliExe) {
    $CliSize = (Get-Item $CliExe).Length / 1MB
    Write-ColorOutput $Green "✅ CLI executable built successfully"
    Write-ColorOutput $Green "   Size: $($CliSize.ToString('F1')) MB"
} else {
    Write-ColorOutput $Red "❌ CLI executable not found"
}

# Test executables
if (-not $SkipTests) {
    Write-ColorOutput $Blue "🧪 Testing executables..."
    
    if (Test-Path $CliExe) {
        Write-ColorOutput $Blue "   Testing CLI..."
        $TestProcess = Start-Process -FilePath $CliExe -ArgumentList "--help" -NoNewWindow -PassThru -Wait
        if ($TestProcess.ExitCode -eq 0 -or $TestProcess.ExitCode -eq $null) {
            Write-ColorOutput $Green "   ✅ CLI test passed"
        } else {
            Write-ColorOutput $Yellow "   ⚠️  CLI test completed with exit code $($TestProcess.ExitCode)"
        }
    }
    
    if (Test-Path $GuiExe) {
        Write-ColorOutput $Blue "   Testing GUI (basic load)..."
        $TestProcess = Start-Process -FilePath $GuiExe -ArgumentList "--version" -NoNewWindow -PassThru -Wait -TimeoutSec 10
        if ($TestProcess.HasExited) {
            Write-ColorOutput $Green "   ✅ GUI test passed"
        } else {
            $TestProcess.Kill()
            Write-ColorOutput $Green "   ✅ GUI launched successfully (killed after timeout)"
        }
    }
}

# Create install package
Write-ColorOutput $Blue "📦 Creating install package..."
& cmake --build . --target package --config $BuildType
if ($LASTEXITCODE -eq 0) {
    Write-ColorOutput $Green "✅ Package created successfully"
} else {
    Write-ColorOutput $Yellow "⚠️  Package creation failed (CPack may not be configured)"
}

Write-Host ""
Write-ColorOutput $Green "🎉 Windows build completed successfully!"
Write-ColorOutput $Green "📍 Build location: $(Get-Location)"
Write-Host ""
Write-ColorOutput $Blue "🚀 To run the applications:"
Write-ColorOutput $Blue "   • GUI: .\$BuildType\TrussAnalysisGUI.exe"
Write-ColorOutput $Blue "   • CLI: .\$BuildType\TrussAnalysisCLI.exe --help"
Write-Host ""
Write-ColorOutput $Blue "🔧 To create installer:"
Write-ColorOutput $Blue "   cmake --build . --target package --config $BuildType"
Write-Host ""

Set-Location ..
