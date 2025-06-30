# Multi-stage Dockerfile for 2D Truss Analysis
# Supports both development and production builds

ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION} AS base

# Install basic dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Development stage with full Qt6 and dependencies
FROM base AS development

# Install Qt6 and development tools
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-charts-dev \
    libeigen3-dev \
    qtbase6-dev-tools \
    clang-format \
    clang-tidy \
    cppcheck \
    gdb \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . .

# Configure and build
RUN cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

RUN cmake --build build --parallel

# Development container entry point
CMD ["/bin/bash"]

# Production stage with minimal runtime
FROM ubuntu:${UBUNTU_VERSION} AS production

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    qt6-base-dev \
    qt6-charts-dev \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Create non-root user
RUN useradd -m -u 1000 trussuser

# Copy built applications from development stage
COPY --from=development /workspace/build/TrussAnalysisGUI /usr/local/bin/
COPY --from=development /workspace/build/TrussAnalysisCLI /usr/local/bin/

# Set permissions
RUN chmod +x /usr/local/bin/TrussAnalysisGUI \
    && chmod +x /usr/local/bin/TrussAnalysisCLI

# Switch to non-root user
USER trussuser

# Set up environment
ENV QT_QPA_PLATFORM=offscreen
ENV DISPLAY=:0

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD TrussAnalysisCLI --help || exit 1

# Default command
CMD ["TrussAnalysisCLI", "--help"]

# Build stage for CI/CD
FROM development AS builder

# Copy build artifacts to a known location
RUN mkdir -p /artifacts && \
    cp build/TrussAnalysisGUI /artifacts/ && \
    cp build/TrussAnalysisCLI /artifacts/ && \
    ls -la /artifacts/

# Entry point for artifact extraction
ENTRYPOINT ["cp", "-r", "/artifacts/", "/output/"]
