#!/bin/bash -x

#### THIS IS THE TOP-LEVEL loudgain SCRIPT ####

DATESTAMP=$(date +'%Y%m%d-%H%M%S')
LOGFILE=$TMP/build-cygwin.$DATESTAMP.log

die() {
    echo "*** ABORTED ***" | tee $LOGFILE
    echo "END BUILD: $(date +'%Y%m%d-%H%M%S')" | tee $LOGFILE
    echo "See $LOGFILE" | tee $LOGFILE
    exit 1
}
export -f die
   
{
    echo "BEGIN BUILD: $DATESTAMP"
    
    SCRIPT=`basename $0 .sh`
    SCRIPT_DIR=$(realpath $(dirname $0))
    BUILD_BRANCH=$(git symbolic-ref --short HEAD)
    
    pushd $SCRIPT_DIR

    rm -rf .install || die
    mkdir -p .install || die

    export CMAKE_INSTALL_PREFIX=$SCRIPT_DIR/.install
    export PKG_CONFIG_PATH=$CMAKE_INSTALL_PREFIX/lib/pkgconfig
    export CMAKE_BUILD_TYPE=${1:-Debug}
    
    export CMAKE_C_FLAGS="-Wno-deprecated-declarations -DTAGLIB_STATIC=1"
    export CMAKE_CXX_FLAGS="-Wno-deprecated-declarations -DTAGLIB_STATIC=1"
    export CFLAGS=$CMAKE_C_FLAGS
    export CXXFLAGS=$CMAKE_CXX_FLAGS
    export LDFLAGS="-L$CMAKE_INSTALL_PREFIX/lib"
    export BUILD_SHARED_LIBS=OFF
    export LD=`which ld`

    cd ../cppunit
    git switch $BUILD_BRANCH || die
    chmod 770 build-cygwin.sh
    ./build-cygwin.sh || die
    
    cd ../libebur128
    git switch $BUILD_BRANCH || die
    chmod 770 build-cygwin.sh
    ./build-cygwin.sh || die
    
    cd ../taglib
    git switch $BUILD_BRANCH || die
    chmod 770 build-cygwin.sh
    ./build-cygwin.sh || die
    
    cd ../ffmpeg
    git switch $BUILD_BRANCH || die
    chmod 770 build-cygwin.sh
    ./build-cygwin.sh || die
    
    cd ../loudgain

    echo "################### BUILDING $0 ####################"
    
    rm -rf .build || die
    mkdir -p .build || die
    cd .build

    cmake --install-prefix=$CMAKE_INSTALL_PREFIX -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS \
        -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_C_FLAGS="$CMAKE_C_FLAGS" \
        -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
        -DCMAKE_STATIC_LINKER_FLAGS=-v -DCMAKE_EXE_LINKER_FLAGS="'$LDFLAGS'" \
        --debug-trycompile -G 'Unix Makefiles' .. || die
    make SHELL='/bin/bash -x' || die
    make install SHELL='/bin/bash -x' || die

    popd

    echo "END BUILD: $(date +'%Y%m%d-%H%M%S')"
    echo "See $LOGFILE"

} 2>&1 | tee $LOGFILE

