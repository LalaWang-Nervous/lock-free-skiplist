rm -rf ./build ./output
mkdir output &&mkdir build && cd build
cmake .. && make
mv ./unit_test ./../output && mv ./performance_test ./../output && mv ./kv_service ./../output
rm -rf ../build