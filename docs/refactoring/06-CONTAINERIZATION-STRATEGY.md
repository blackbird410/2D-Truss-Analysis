# Containerization Strategy: 2D Truss Analysis

**Date:** February 4, 2026  
**Target Platform:** Linux (Ubuntu 22.04 base)  
**Container Registry:** Docker Hub / GitHub Container Registry

---

## 1. Container Design Philosophy

**Objectives:**

1. **Minimal Size** - Target < 500MB final image
2. **Security** - Non-root user, minimal attack surface
3. **Reproducibility** - Consistent builds across environments
4. **Performance** - Optimized layer caching
5. **Maintainability** - Clear, documented Dockerfile

---

## 2. Multi-Stage Dockerfile

### 2.1 Optimized Dockerfile

```dockerfile
# =============================================================================
# Stage 1: Base image with system dependencies
# =============================================================================
FROM ubuntu:22.04 AS base

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
    && rm -rf /var/lib/apt/lists/*

# Set build directory
WORKDIR /build

# Copy only build-related files first (better caching)
COPY CMakeLists.txt ./
COPY cmake/ ./cmake/
COPY include/ ./include/

# Copy source code
COPY src/ ./src/

# Build application
RUN cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX=/opt/truss \
    && cmake --build build -j$(nproc) \
    && cmake --install build

# =============================================================================
# Stage 3: Runtime image (minimal)
# =============================================================================
FROM base AS runtime

# No additional runtime dependencies required (Eigen is header-only and
# already compiled into the binary in the builder stage)

# Create non-root user
RUN useradd -m -u 1000 -s /bin/bash truss

# Copy built application from builder
COPY --from=builder /opt/truss /opt/truss

# Copy runtime configuration
COPY --chown=truss:truss config/ /opt/truss/config/

# Set up runtime environment
ENV PATH="/opt/truss/bin:${PATH}"
ENV TRUSS_CONFIG_PATH="/opt/truss/config"

# Create data directories
RUN mkdir -p /data/input /data/output \
    && chown -R truss:truss /data

# Switch to non-root user
USER truss
WORKDIR /data

# Health check (if applicable)
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD TrussAnalysisCLI --version || exit 1

# Default command
ENTRYPOINT ["TrussAnalysisCLI"]
CMD ["--help"]

# Metadata
LABEL org.opencontainers.image.title="2D Truss Analysis"
LABEL org.opencontainers.image.description="Professional 2D truss structural analysis tool"
LABEL org.opencontainers.image.version="3.0.0"
LABEL org.opencontainers.image.authors="Neil Taison Rigaud <ntsrigaud14@gmail.com>"
LABEL org.opencontainers.image.source="https://github.com/blackbird410/2D-Truss-Analysis-cpp"
LABEL org.opencontainers.image.licenses="MIT"
```

### 2.2 Development Dockerfile

```dockerfile
# Dockerfile.dev - Development environment with debugging tools
FROM ubuntu:22.04 AS dev

ENV DEBIAN_FRONTEND=noninteractive

# Install development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    libeigen3-dev \
    gdb \
    valgrind \
    clang-format \
    clang-tidy \
    cppcheck \
    git \
    vim \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Non-root user for development
RUN useradd -m -u 1000 -s /bin/bash dev \
    && chown -R dev:dev /workspace

USER dev

CMD ["/bin/bash"]
```

---

## 3. Docker Compose Configuration

### 3.1 docker-compose.yml

```yaml
version: "3.8"

services:
  truss-analysis:
    build:
      context: .
      dockerfile: docker/Dockerfile
      target: runtime
    image: truss-analysis:latest
    container_name: truss-analysis
    volumes:
      - ./examples:/data/input:ro
      - ./output:/data/output:rw
    environment:
      - TRUSS_LOG_LEVEL=info
    user: "1000:1000"

  truss-dev:
    build:
      context: .
      dockerfile: docker/Dockerfile.dev
    image: truss-analysis:dev
    container_name: truss-dev
    volumes:
      - .:/workspace:rw
      - build-cache:/workspace/build
    working_dir: /workspace
    stdin_open: true
    tty: true
    command: /bin/bash

volumes:
  build-cache:
```

---

## 4. .dockerignore

```gitignore
# Build artifacts
build/
build_*/
*.o
*.a
*.so

# IDE files
.vscode/
.idea/
*.swp

# Git
.git/
.gitignore

# Documentation (not needed in container)
docs/

# Tests (not needed in runtime)
tests/

# CI/CD
.github/

# Temporary files
*.tmp
*.log
*.bak

# macOS
.DS_Store

# Keep necessary files
!README.md
!LICENSE
```

---

## 5. Container Usage Patterns

### 5.1 Basic Usage

```bash
# Build container
docker build -t truss-analysis:latest -f docker/Dockerfile .

# Run analysis on a file
docker run --rm \
    -v $(pwd)/examples:/data/input:ro \
    -v $(pwd)/output:/data/output:rw \
    truss-analysis:latest \
    --input /data/input/simple-truss.json \
    --output /data/output/results.csv \
    --format csv

# Interactive mode
docker run --rm -it \
    -v $(pwd)/data:/data \
    truss-analysis:latest \
    --interactive
```

### 5.2 Development Workflow

```bash
# Start development container
docker-compose run --rm truss-dev

# Inside container:
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/TrussAnalyze --example
```

### 5.3 CI/CD Integration

```yaml
# .github/workflows/docker-build.yml
name: Docker Build and Push

on:
  push:
    tags:
      - "v*"

jobs:
  docker:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v2

      - name: Login to GitHub Container Registry
        uses: docker/login-action@v2
        with:
          registry: ghcr.io
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}

      - name: Extract metadata
        id: meta
        uses: docker/metadata-action@v4
        with:
          images: ghcr.io/${{ github.repository }}
          tags: |
            type=semver,pattern={{version}}
            type=semver,pattern={{major}}.{{minor}}
            type=sha

      - name: Build and push
        uses: docker/build-push-action@v4
        with:
          context: .
          file: ./docker/Dockerfile
          push: true
          tags: ${{ steps.meta.outputs.tags }}
          labels: ${{ steps.meta.outputs.labels }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
```

---

## 6. Container Optimization Techniques

### 6.1 Layer Caching Strategy

**Order of operations (most to least frequently changing):**

1. System dependencies (rarely change)
2. Build tools (rarely change)
3. Application dependencies (occasionally change)
4. Source code (frequently changes)

### 6.2 Image Size Optimization

**Current size:** ~800MB (before optimization)  
**Target size:** <500MB

**Optimization techniques:**

1. **Multi-stage builds** - Separate builder and runtime
2. **Minimal base** - Ubuntu instead of full distribution
3. **Cleanup** - Remove package lists after install
4. **Combined RUN commands** - Reduce layer count
5. **No build artifacts** - Only copy final binary
6. **Strip binaries** - Remove debug symbols in release

### 6.3 Build Time Optimization

```bash
# Use BuildKit for better caching
export DOCKER_BUILDKIT=1

# Build with cache mounts (faster rebuilds)
docker build \
    --cache-from=truss-analysis:cache \
    --build-arg BUILDKIT_INLINE_CACHE=1 \
    -t truss-analysis:latest \
    -f docker/Dockerfile .
```

---

## 7. Security Best Practices

### 7.1 Security Checklist

- [x] Non-root user
- [x] Minimal base image (Ubuntu 22.04)
- [x] No unnecessary packages
- [x] Read-only input volumes
- [x] Explicit USER directive
- [x] Health checks
- [x] Container labels
- [x] No secrets in image
- [x] Regular base image updates

### 7.2 Vulnerability Scanning

```bash
# Scan for vulnerabilities
docker scan truss-analysis:latest

# Or use Trivy
trivy image truss-analysis:latest
```

---

## 8. Container Deployment Scenarios

### 8.1 Local Development

```bash
# Quick analysis
docker run --rm -v $(pwd):/data truss-analysis:latest \
    --input /data/model.json \
    --output /data/results.csv
```

### 8.2 CI/CD Pipeline

```yaml
# Use container in GitHub Actions
jobs:
  analyze:
    runs-on: ubuntu-latest
    container: ghcr.io/blackbird410/truss-analysis:latest
    steps:
      - uses: actions/checkout@v3
      - name: Run analysis
        run: TrussAnalyze --input model.json --output results.csv
```

### 8.3 Cloud Deployment (AWS, Azure, GCP)

```bash
# AWS ECS Task Definition
{
  "family": "truss-analysis",
  "containerDefinitions": [{
    "name": "truss-analysis",
    "image": "ghcr.io/blackbird410/truss-analysis:latest",
    "memory": 512,
    "cpu": 256,
    "essential": true,
    "environment": [
      {"name": "TRUSS_LOG_LEVEL", "value": "info"}
    ],
    "logConfiguration": {
      "logDriver": "awslogs",
      "options": {
        "awslogs-group": "/ecs/truss-analysis",
        "awslogs-region": "us-east-1"
      }
    }
  }]
}
```

### 8.4 Kubernetes Deployment

```yaml
# k8s-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: truss-analysis
spec:
  replicas: 3
  selector:
    matchLabels:
      app: truss-analysis
  template:
    metadata:
      labels:
        app: truss-analysis
    spec:
      containers:
        - name: truss-analysis
          image: ghcr.io/blackbird410/truss-analysis:3.0.0
          resources:
            requests:
              memory: "256Mi"
              cpu: "250m"
            limits:
              memory: "512Mi"
              cpu: "500m"
          volumeMounts:
            - name: data
              mountPath: /data
      volumes:
        - name: data
          persistentVolumeClaim:
            claimName: truss-data-pvc
```

---

## 9. Container Testing

### 9.1 Container Tests

```bash
# Test container builds
docker build -t truss-analysis:test -f docker/Dockerfile .

# Test container runs
docker run --rm truss-analysis:test --version

# Test with sample data
docker run --rm \
    -v $(pwd)/tests/fixtures/sample_data:/data:ro \
    truss-analysis:test \
    --input /data/simple_truss.json

# Test resource limits
docker run --rm -m 512m --cpus=1 truss-analysis:test --example
```

### 9.2 Container Structure Tests

```bash
# Use container-structure-test
container-structure-test test \
    --image truss-analysis:latest \
    --config tests/container-test.yaml
```

```yaml
# tests/container-test.yaml
schemaVersion: "2.0.0"

fileExistenceTests:
  - name: "Binary exists"
    path: "/opt/truss/bin/TrussAnalyze"
    shouldExist: true
    permissions: "-rwxr-xr-x"

  - name: "Config directory exists"
    path: "/opt/truss/config"
    shouldExist: true

commandTests:
  - name: "Version check"
    command: "TrussAnalyze"
    args: ["--version"]
    exitCode: 0

  - name: "Help command"
    command: "TrussAnalyze"
    args: ["--help"]
    exitCode: 0

metadataTest:
  labels:
    - key: "org.opencontainers.image.title"
      value: "2D Truss Analysis"
    - key: "org.opencontainers.image.version"
      value: "3.0.0"
```

---

## 10. Makefile Integration

```makefile
# Makefile targets for Docker operations

.PHONY: docker-build docker-run docker-dev docker-test docker-push

DOCKER_IMAGE := truss-analysis
DOCKER_TAG := latest
REGISTRY := ghcr.io/blackbird410

docker-build:
	docker build -t $(DOCKER_IMAGE):$(DOCKER_TAG) -f docker/Dockerfile .

docker-run:
	docker run --rm -it \
		-v $(PWD)/examples:/data/input:ro \
		-v $(PWD)/output:/data/output:rw \
		$(DOCKER_IMAGE):$(DOCKER_TAG)

docker-dev:
	docker-compose run --rm truss-dev

docker-test:
	docker build -t $(DOCKER_IMAGE):test -f docker/Dockerfile .
	docker run --rm $(DOCKER_IMAGE):test --version
	container-structure-test test \
		--image $(DOCKER_IMAGE):test \
		--config tests/container-test.yaml

docker-push:
	docker tag $(DOCKER_IMAGE):$(DOCKER_TAG) $(REGISTRY)/$(DOCKER_IMAGE):$(DOCKER_TAG)
	docker push $(REGISTRY)/$(DOCKER_IMAGE):$(DOCKER_TAG)

docker-clean:
	docker rmi $(DOCKER_IMAGE):$(DOCKER_TAG) || true
	docker system prune -f
```

---

## 11. Image Size Comparison

| Stage                                | Size       | Reduction |
| ------------------------------------ | ---------- | --------- |
| **Full Ubuntu + Qt6 + dev tools**    | ~2.5GB     | Baseline  |
| **Ubuntu + CLI only + runtime deps** | ~800MB     | 68%       |
| **Multi-stage + optimization**       | **~450MB** | **82%**   |

---

## 12. Implementation Checklist

- [ ] Create optimized Dockerfile
- [ ] Create .dockerignore
- [ ] Create docker-compose.yml
- [ ] Create development Dockerfile
- [ ] Implement health checks
- [ ] Configure non-root user
- [ ] Set up layer caching
- [ ] Add container labels
- [ ] Write container tests
- [ ] Integrate with Makefile
- [ ] Set up CI/CD for container builds
- [ ] Document container usage
- [ ] Optimize image size < 500MB
- [ ] Security scan passing

---

## 13. Success Criteria

✅ **Image size < 500MB**  
✅ **Build time < 5 minutes** (cached < 30 seconds)  
✅ **Non-root execution**  
✅ **Security scan passes**  
✅ **Container tests pass**  
✅ **CI/CD integration complete**  
✅ **Documentation complete**

---

**Document Status:** Ready for Implementation  
**Next:** Proceed to Build System Design
