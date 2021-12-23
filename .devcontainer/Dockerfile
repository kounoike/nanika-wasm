ARG VARIANT=ubuntu-20.04
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
  && apt-get -y install --no-install-recommends \
  ca-certificates \
  firefox \
  fonts-liberation \
  gnupg \
  libappindicator1 \
  libdrm2 \
  libgbm1 \
  libjpeg-dev \
  libgtest-dev \
  libnss3 \
  libpng-dev \
  ninja-build \
  python \
  python3-dev \
  python3-numpy \
  xdg-utils \
  libasound2 \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

RUN curl -O https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb && \
  sudo dpkg -i google-chrome-stable_current_amd64.deb && \
  rm google-chrome-stable_current_amd64.deb

ARG EMSCRIPTEN_VERSION=3.0.0
# ### EMSCRIPTEN for base
WORKDIR /
RUN git clone https://github.com/emscripten-core/emsdk.git --depth 1
WORKDIR /emsdk
RUN ./emsdk install ${EMSCRIPTEN_VERSION} && ./emsdk activate ${EMSCRIPTEN_VERSION}
RUN echo '. /emsdk/emsdk_env.sh > /dev/null 2>&1' >> /home/vscode/.bashrc
