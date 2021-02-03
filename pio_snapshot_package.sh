#!/bin/bash

set -x -e -v

git restore --source HEAD -- cores/esp8266/core_version.h
git restore --source HEAD -- package.json

version=$(git rev-parse --short=8 HEAD)
desc=$(git describe --tags)

cat <<EOF > package.json
{
    "name": "framework-arduinoespressif8266",
    "description": "ESP8266 Core - Current git version snapshot + some patches",
    "version": "3.0.0-git${version}",
    "repository": {
        "type": "git",
        "url": "https://github.com/mcspr/esp8266-Arduino",
        "branch": "${version}"
    }
}
EOF

cat <<EOF > cores/esp8266/core_version.h
#define ARDUINO_ESP8266_GIT_VER 0x$version
#define ARDUINO_ESP8266_GIT_DESC $desc
EOF

zip -v -r framework-arduinoespressif8266-git$version.zip . \
    -x "framework-arduinoespressif8266-git$version.zip" \
    -x '.gitignore' \
    -x '.gitmodules' \
    -x '.git/*' \
    -x 'doc/*' \
    -x 'package/*' \
    -x 'tools/esptool/*' \
    -x 'tools/pyserial/*' \
    -x '**/doc/*' \
    -x '**/.git/*' \
    -x '**/.github/*' \
    -x '**/.gitignore' \
    -x '**/.gitmodules'
