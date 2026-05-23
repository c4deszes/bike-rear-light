FROM ghcr.io/c4deszes/arm-gcc-docker:v1.4.0

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    build-essential \
    gcc \
    g++ \
    git \
    doxygen \
    graphviz \
    plantuml

RUN wget -O drawio.deb https://github.com/jgraph/drawio-desktop/releases/download/v29.3.6/drawio-amd64-29.3.6.deb
RUN apt-get install -y xvfb libnotify4 xdg-utils libsecret-1-0 libappindicator3-1
RUN apt-get install -y ./drawio.deb
