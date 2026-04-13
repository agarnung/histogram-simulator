# histogram-simulator
An application to perform LUT-based image histogram modification interactively

<p align="center">
  <img src="./assets/simulator.png" alt="Simulator overview" title="Simulator overview" />
</p>

This project uses [OpenCV 4](https://github.com/opencv/opencv/tree/4.10.0) (see additional packages in https://docs.opencv.org/3.4/d7/d9f/tutorial_linux_install.html)

It is recommended build OpenCV with `-DOPENCV_GENERATE_PKGCONFIG=ON`, see [this](https://stackoverflow.com/questions/15320267/package-opencv-was-not-found-in-the-pkg-config-search-path)

References:
- Bézier curves approximation thanks to the work of [Maxim Shemanarev](https://agg.sourceforge.net/antigrain.com/research/adaptive_bezier/)

_TODO_:

- The idea will be binding all required libraries into a single AppImage, in order to ship the application directly
- Refractor directories in modules
- Add safety in TRANSFORM button and other places

---

**Local (Linux):** install `build-essential`, `qtbase5-dev`, `qttools5-dev-tools`, `pkg-config`, `libopencv-dev`; then `mkdir build && cd build && qmake CONFIG+=release ../histogramsimulator.pro && make -j$(nproc) && ./HistogramSimulator`.

**Docker:** `docker compose build && docker compose run --rm app` (GUI needs WSLg or X11; use `DISPLAY=:1 docker compose run --rm app` if not `:0`). The compose bind-mounts this repo to `/opt/histogram-simulator`—put images under the project on disk and open them from that path (e.g. `/opt/histogram-simulator/...`) in the file dialog.
