# syntax=docker/dockerfile:1
# BuildKit: caché de apt y de compilación (rebuilds mucho más rápidas).
#   DOCKER_BUILDKIT=1 docker compose build
FROM ubuntu:24.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive \
    PATH="/usr/lib/ccache:${PATH}"
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ccache \
        qtbase5-dev \
        qttools5-dev-tools \
        pkg-config \
        libopencv-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
# Capas estáticas primero (mejor caché al tocar solo .cpp)
COPY histogramsimulator.pro ./
COPY assets/ ./assets/
COPY data/ ./data/
COPY app/ ./app/
COPY main.cpp ./
RUN sed -i 's|/usr/local/include/opencv4|/usr/include/opencv4|g' histogramsimulator.pro
RUN --mount=type=cache,target=/root/.ccache \
    mkdir -p build \
    && cd build \
    && qmake CONFIG+=release ../histogramsimulator.pro \
    && make -j"$(nproc)" \
    && install -m755 HistogramSimulator /usr/local/bin/HistogramSimulator

FROM ubuntu:24.04
# Runtime OpenCV: solo lo que enlaza el binario (core/imgproc/imgcodecs).
ENV DEBIAN_FRONTEND=noninteractive
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        libqt5widgets5 \
        libqt5gui5 \
        libqt5core5a \
        libqt5svg5 \
        libglib2.0-0 \
        libfontconfig1 \
        libfreetype6 \
        libxcb-xinerama0 \
        libxcb-cursor0 \
        libxkbcommon-x11-0 \
        libdbus-1-3 \
        libxcb1 \
        libx11-6 \
        libx11-xcb1 \
        libsm6 \
        libice6 \
        libgl1 \
        libqt5dbus5 \
        libopencv-core406t64 \
        libopencv-imgproc406t64 \
        libopencv-imgcodecs406t64 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=builder /usr/local/bin/HistogramSimulator /usr/local/bin/HistogramSimulator
# opencv4.pc trae muchos -l; con --as-needed solo quedan las .so usadas (suele ser core+imgproc+imgcodecs).
RUN ! ldd /usr/local/bin/HistogramSimulator | grep -q 'not found'
WORKDIR /opt/histogram-simulator
ENV QT_QPA_PLATFORM=xcb
ENTRYPOINT ["/usr/local/bin/HistogramSimulator"]
