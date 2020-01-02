rm -r dist && mkdir -p dist

export code_dir=/home/killf/dlab/FaceVideo
export ffmpeg_dir=/home/sensing/github/ffmpeg

cd $ffmpeg_dir
emmake make clean
emconfigure ./configure --cc="emcc" --cxx="em++" --ar="emar" --prefix=$code_dir/dist --enable-cross-compile --target-os=none \
        --arch=x86_32 --cpu=generic --enable-gpl --enable-version3 --disable-avdevice --disable-swresample --disable-postproc --disable-avfilter \
        --disable-programs --disable-logging --disable-everything --enable-avformat --enable-decoder=hevc --enable-decoder=h264 --enable-decoder=aac \
        --disable-ffplay --disable-ffprobe --disable-ffserver --disable-asm --disable-doc --disable-devices --disable-network --disable-hwaccels \
        --disable-parsers --disable-bsfs --disable-debug --enable-protocol=file --enable-demuxer=mov --enable-demuxer=mpgets --disable-indevs --disable-outdevs

emmake make -j8
emmake make install
