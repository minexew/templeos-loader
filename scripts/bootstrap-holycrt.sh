#!/bin/sh

mkdir -p CmpOutput

env STARTOS=D:/HolyCRT/CmpHolyCRT.HC \
    ./cmake-build-debug/templeos-loader MiniSystem/Kernel/HolyCRT.BIN \
    --drive=C,MiniSystem \
    --drive=D,.,CmpOutput
