rm -rf test.html
export TOTAL_MEMORY=671088640
export EXPORTED_FUNCTIONS="['_main']"

echo "Running Emscripten..."
emcc main.cpp lib/lib/libavformat.a lib/lib/libavcodec.a lib/lib/libavutil.a lib/lib/libswscale.a \
    -O0 \
    -I "lib/include" \
    -s WASM=1 \
    -s DEMANGLE_SUPPORT=1 \
    -s TOTAL_MEMORY=${TOTAL_MEMORY} \
    -s EXPORTED_FUNCTIONS="${EXPORTED_FUNCTIONS}" \
    -s EXTRA_EXPORTED_RUNTIME_METHODS="['addFunction']" \
    -s RESERVED_FUNCTION_POINTERS=14 \
    -s FORCE_FILESYSTEM=1 \
    -s USE_SDL=2 \
    -s USE_SDL_IMAGE=2 \
    -s SDL2_IMAGE_FORMATS='["png","bmp","jpg"]' \
    --preload-file videos \
    -o test.html

echo "Finished Build"
