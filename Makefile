
debug:
	mkdir -p build/debug && cd build/debug && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../../ && make -j8

release:
	mkdir -p build/release && cd build/release && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ../../ && make -j8
