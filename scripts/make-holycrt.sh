#!/bin/sh

mkdir -p CmpOutput

env STARTOS=D:/HolyCRT/CmpHolyCRT.HC \
    ./cmake-build-debug/templeoskernel \
    --drive=C,MiniSystem \
    --drive=D,.,CmpOutput
