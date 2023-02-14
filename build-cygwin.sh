#!/bin/bash -x

#### THIS IS THE TOP-LEVEL loudgain SCRIPT ####

DATESTAMP=$(date +'%Y%m%d-%H%M%S')

{
    echo "BEGIN BUILD: $DATESTAMP"
    
    SCRIPT=`basename $0 .sh`
    SCRIPT_DIR=$(realpath $(dirname $0))
    
    pushd $SCRIPT_DIR

    # rm -rf .install || exit 1
    mkdir -p .install || exit 1

    export CMAKE_INSTALL_PREFIX=$SCRIPT_DIR/.install
    export PKG_CONFIG_PATH=$CMAKE_INSTALL_PREFIX/lib/pkgconfig
    export CMAKE_BUILD_TYPE=${1:-Debug}
    
    export CMAKE_C_FLAGS=-Wno-deprecated-declarations
    export CMAKE_CXX_FLAGS=-Wno-deprecated-declarations
    export CFLAGS=$CMAKE_C_FLAGS
    export CXXFLAGS=$CMAKE_CXX_FLAGS
    export LDFLAGS="-L$CMAKE_INSTALL_PREFIX/lib"
    export BUILD_SHARED_LIBS=OFF

    chmod 770 ../cppunit/build-cygwin.sh
    ../cppunit/build-cygwin.sh || exit 1
    chmod 770 ../libebur128/build-cygwin.sh
    ../libebur128/build-cygwin.sh || exit 1
    chmod 770 ../taglib/build-cygwin.sh
    ../taglib/build-cygwin.sh || exit 1
    chmod 770 ../ffmpeg/build-cygwin.sh
    ../ffmpeg/build-cygwin.sh || exit 1

    echo "################### BUILDING $0 ####################"
    
    rm -rf .build || exit 1
    mkdir -p .build || exit 1
    cd .build

    cmake --install-prefix=$CMAKE_INSTALL_PREFIX -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS \
        -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_C_FLAGS=$CMAKE_C_FLAGS -DCMAKE_CXX_FLAGS=$CMAKE_CXX_FLAGS \
        -DCMAKE_STATIC_LINKER_FLAGS=-v -DCMAKE_EXE_LINKER_FLAGS="'$LDFLAGS'" \
        --debug-trycompile -G 'Unix Makefiles' .. || exit 1
    make SHELL='/bin/bash -x' || exit 1
    make install SHELL='/bin/bash -x' || exit 1

    popd

    echo "END BUILD: $(date +'%Y%m%d-%H%M%S')"

} 2>&1 | tee $TMP/build-cygwin.$DATESTAMP.log

