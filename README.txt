bnav

Decodes BeiDou navigation messages from Septentrio SBF (and JPS) ASCII
exports and writes IONEX ionosphere maps.

Dependencies

* g++ 13+ or clang 18+ (C++23)
* Boost.Program_options (1.83 tested)
* qmake from Qt 6 (the project itself does not link Qt)
* UnitTest++ for the tests

Build

    mkdir build && cd build
    qmake6 ../bnav3.pro
    make -j$(nproc)

Binaries are build/src/bapp and build/tests/tests. RPATH points at
build/lib, so LD_LIBRARY_PATH is not required.

    ./tests/tests
    ./src/bapp --help
    ./src/bapp --sv 2 --file ../tests/data/sbf/CUT12014071724.sbf_SBF_CMPRaw-prn2.txt \
        --klobuchar klob.ionex --regional regional.ionex

Files that contain both B1 and B2 rows must be filtered to one signal
before decoding; interleaved signals do not form continuous subframes.
