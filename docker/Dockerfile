# =============================================================================
# Multi-Stage Dockerfile for 2D Truss Analysis
# Optimized for minimal size and security
# Target: CLI-only runtime < 500MB
# =============================================================================

# =============================================================================
# Stage 1: Base image with system dependencies
# =============================================================================
FROM ubuntu:22.04.3 AS base

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# =============================================================================
# Stage 2: Builder image with build tools
# =============================================================================
FROM base AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    libeigen3-dev \
    nlohmann-json3-dev \
    libtinyxml2-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Set build directory
WORKDIR /build

# Copy only build-related files first (better caching)
COPY CMakeLists.txt ./
COPY cmake/ ./cmake/
COPY vcpkg.json ./
COPY LICENSE ./
COPY README.md ./

# Copy source code
COPY src/ ./src/

# Build application (CLI only, no Qt6 dependencies)
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DBUILD_GUI=OFF \
    -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
    && cmake --build build --target TrussAnalysisCLI -j$(nproc)

# Install the binary and strip it
RUN mkdir -p /opt/truss/bin && \
    cp build/TrussAnalysisCLI /opt/truss/bin/ && \
    strip /opt/truss/bin/TrussAnalysisCLI

# =============================================================================
# Stage 3: Runtime image (minimal)
# =============================================================================
FROM base AS runtime

# Install only minimal runtime dependencies
# Eigen is header-only, nlohmann_json is header-only, they're compiled into binary
RUN apt-get update && apt-get install -y \
    --no-install-recommends \
    libtinyxml2-9 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 -s /bin/bash truss

# Copy built application from builder
COPY --from=builder /opt/truss/bin/TrussAnalysisCLI /usr/local/bin/

# Set executable permissions
RUN chmod +x /usr/local/bin/TrussAnalysisCLI

# Create data directories
RUN mkdir -p /data/input /data/output \
    && chown -R truss:truss /data

# Switch to non-root user
USER truss
WORKDIR /data

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD TrussAnalysisCLI --help > /dev/null 2>&1 || exit 1

# Default command
ENTRYPOINT ["TrussAnalysisCLI"]
CMD ["--help"]

# Metadata
LABEL org.opencontainers.image.title="2D Truss Analysis"
LABEL org.opencontainers.image.description="Professional 2D truss structural analysis tool (CLI)"
LABEL org.opencontainers.image.version="3.0.0"
LABEL org.opencontainers.image.authors="Neil Taison Rigaud <ntsrigaud14@gmail.com>"
LABEL org.opencontainers.image.source="https://github.com/blackbird410/2D-Truss-Analysis-cpp"
LABEL org.opencontainers.image.licenses="MIT"
