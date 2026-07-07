#!/bin/bash

set -e
set -x

ARTIFACTS=${ARTIFACTS:-"artifacts"}
DESTINATION=${DESTINATION:-"release"}
VERSION=${VERSION:-"extension"}
ADDON_NAME=${ADDON_NAME:-"webrtc_native"}

mkdir -p ${DESTINATION}
ls -R ${DESTINATION}
ls -R ${ARTIFACTS}

DESTDIR="${DESTINATION}/${VERSION}/addons/${ADDON_NAME}"

mkdir -p ${DESTDIR}/lib

find "${ARTIFACTS}" -maxdepth 6 -wholename "*/${VERSION}/addons/${ADDON_NAME}/lib/*" | xargs cp -r -t "${DESTDIR}/lib/"
find "${ARTIFACTS}" -wholename "*/LICENSE*" | xargs cp -t "${DESTDIR}/"

if [ $VERSION = "gdnative" ]; then
    find "${ARTIFACTS}" -wholename "*/${VERSION}/addons/${ADDON_NAME}/${ADDON_NAME}.tres" | head -n 1 | xargs cp -t "${DESTDIR}/"
else
    find "${ARTIFACTS}" -wholename "*/${VERSION}/addons/${ADDON_NAME}/${ADDON_NAME}.gdextension" | head -n 1 | xargs cp -t "${DESTDIR}/"
fi

CURDIR=$(pwd)
cd "${DESTINATION}/${VERSION}"
zip -r ../godot-${VERSION}-${ADDON_NAME}.zip addons
cd "$CURDIR"

ls -R ${DESTINATION}
