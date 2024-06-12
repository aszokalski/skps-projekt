scripts/feeds update skps
scripts/feeds install -p skps -a
export LANG=C
make package/lidar/compile -j1 V=s
cd bin/packages/aarch64_cortex-a72/skps
ls
python3 -m http.server 