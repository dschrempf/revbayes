#!/bin/sh
./regenerate.sh -MAC_universal true
cmake . -DCMAKE_BUILD_TYPE=release "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386"
make 