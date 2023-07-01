#!/bin/bash

set -e
set -x

ARTIFACTS=${ARTIFACTS:-"artifacts"}
DESTINATION=${DESTIONATION:-"release"}
VERSION=${VERSION:-"extension"}
TYPE=${TYPE:-"webrtc"}

mkdir -p ${DESTINATION}
ls -R ${DESTINATION}
ls -R ${ARTIFACTS}

DESTDIR="${DESTINATION}/${VERSION}/${TYPE}"

mkdir -p ${DESTDIR}/lib

find "${ARTIFACTS}" -wholename "*/${VERSION}/${TYPE}/lib/*" | xargs cp -t "${DESTDIR}/lib/"
find "${ARTIFACTS}" -wholename "*/LICENSE*" | xargs cp -t "${DESTDIR}/"

if [ $VERSION = "gdnative" ]; then
    find "${ARTIFACTS}" -wholename "*/${VERSION}/${TYPE}/${TYPE}.tres" | head -n 1 | xargs cp -t "${DESTDIR}/"
else
    find "${ARTIFACTS}" -wholename "*/${VERSION}/${TYPE}/${TYPE}.gdextension" | head -n 1 | xargs cp -t "${DESTDIR}/"
fi

CURDIR=$(pwd)
cd "${DESTINATION}/${VERSION}"
zip -r ../godot-${VERSION}-${TYPE}.zip ${TYPE}
cd "$CURDIR"

ls -R ${DESTINATION}
