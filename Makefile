# Здесь в основном будут описаны шорткаты для пользователя
# А-ля build, test(с параметром при необходимости), clean и sanitized

SANITIZE ?=
CMAKE_FLAGS = -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=20

ifneq ($(SANITIZE),)
	CMAKE_FLAGS += -DCMAKE_CXX_FLAGS="-fsanitize=$(SANITIZE) -fno-omit-frame-pointer" \
                                    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=$(SANITIZE)"
endif

.PHONY: all build test clean sanitized

all: build test

build:
	cmake -B build $(CMAKE_FLAGS)
	cmake --build build --parallel 2

test: build
	cd build && ctest --output-on-failure --parallel 2

sanitized:
	$(MAKE) clean
	$(MAKE) test SANITIZE=address,undefined


clean:
	rm -rf build