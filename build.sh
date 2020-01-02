rm -rf test.html
export TOTAL_MEMORY=67108864
export EXPORTED_FUNCTIONS="['_main']"

echo "Running Emscripten..."
emcc main.c -L/home/killf/dlab/FaceVideo/dist/lib dist/lib/libavformat.a dist/lib/libavcodec.a dist/lib/libavutil.a dist/lib/libswscale.a \
    -O0 -g \
    -I "dist/include" \
    -I "/home/killf/.emscripten_cache/wasm-obj/include" \
    -s WASM=1 \
    -s DEMANGLE_SUPPORT=1 \
    -s TOTAL_MEMORY=${TOTAL_MEMORY} \
    -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy \
    -s EXPORTED_FUNCTIONS="${EXPORTED_FUNCTIONS}" \
    -s EXTRA_EXPORTED_RUNTIME_METHODS="['addFunction']" \
    -s RESERVED_FUNCTION_POINTERS=14 \
    -s FORCE_FILESYSTEM=1 \
    -s USE_SDL=2 \
    -s USE_SDL_IMAGE=2 \
    -s SDL2_IMAGE_FORMATS='["png","bmp","jpg"]' \
    --preload-file videos \
    -s ASYNCIFY=1 -s ASYNCIFY_STACK_SIZE=409600 \
    -s USE_PTHREADS=0 \
    -o test.html

echo "Finished Build"
