# ffmpeg-cpp
#
#   make                    Build everything
#   make <example>          Build (if needed) and run an example
#   make test               Run unit tests
#   make help               Show all targets
#
# Windows: run from Git Bash, MSYS2, or WSL.

VCPKG_ROOT ?= $(HOME)/vcpkg
BUILD_DIR  ?= build
CONFIG     ?= Release
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

EXAMPLES := print_info decode_audio decode_video encode_audio encode_video \
            filtering_audio filtering_video remuxing difference demo simple_interface_demo

VCPKG_BIN       := $(VCPKG_ROOT)/vcpkg
TOOLCHAIN       := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
VCPKG_INSTALLED := $(CURDIR)/vcpkg_installed

ifeq ($(OS),Windows_NT)
EXT   := .exe
SHELL := /bin/bash
else
EXT   :=
endif

.DEFAULT_GOAL := all

.PHONY: all build clean run test help $(EXAMPLES)

## ---------------------------------------------------------------------------
## Default: build everything
## ---------------------------------------------------------------------------
all: build
	@echo ""
	@echo "Done. Run an example with:  make demo"

## ---------------------------------------------------------------------------
## Setup — install vcpkg dependencies (Catch2)
## ---------------------------------------------------------------------------
setup:
	@echo "Checking for vcpkg..."
	@if [ ! -d "$(VCPKG_ROOT)" ]; then \
		echo "Cloning vcpkg to $(VCPKG_ROOT)..."; \
		git clone https://github.com/microsoft/vcpkg "$(VCPKG_ROOT)" || { \
			echo "ERROR: failed to clone vcpkg."; \
			exit 1; \
		}; \
	fi
	@if [ ! -f "$(VCPKG_BIN)" ] && [ ! -f "$(VCPKG_BIN).exe" ]; then \
		echo "Bootstrapping vcpkg..."; \
		if [ -f "$(VCPKG_ROOT)/bootstrap-vcpkg.sh" ]; then \
			"$(VCPKG_ROOT)/bootstrap-vcpkg.sh" || { echo "ERROR: bootstrap failed."; exit 1; }; \
		elif [ -f "$(VCPKG_ROOT)/bootstrap-vcpkg.bat" ]; then \
			"$(VCPKG_ROOT)/bootstrap-vcpkg.bat" || { echo "ERROR: bootstrap failed."; exit 1; }; \
		else \
			echo "ERROR: no bootstrap script found."; \
			exit 1; \
		fi; \
	fi
	@echo "Installing vcpkg dependencies (Catch2)..."
	@export PATH="$(VCPKG_ROOT):$$PATH"; \
	"$(VCPKG_BIN)" install || { \
		echo "ERROR: vcpkg install failed."; \
		exit 1; \
	}
	@echo "vcpkg ready at $(VCPKG_ROOT)"

## ---------------------------------------------------------------------------
## Build — system FFmpeg via pkg-config
## ---------------------------------------------------------------------------
build:
	@if pkg-config --exists libavcodec libavformat libavutil libswscale libswresample 2>/dev/null; then \
		cmake -B $(BUILD_DIR) \
			-DCMAKE_BUILD_TYPE=$(CONFIG) \
			$(EXTRA) || { \
			echo "ERROR: CMake configure failed."; \
			exit 1; \
		}; \
	else \
		echo "ERROR: FFmpeg not found via pkg-config."; \
		echo "  Linux:  sudo dnf install ffmpeg-devel   (Fedora)"; \
		echo "          sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev libavfilter-dev"; \
		echo "  macOS:  brew install ffmpeg"; \
		exit 1; \
	fi
	cmake --build $(BUILD_DIR) --config $(CONFIG) -j$(JOBS) || { \
		echo "ERROR: build failed."; \
		exit 1; \
	}

## ---------------------------------------------------------------------------
## Run examples — auto-builds if binary is missing
## ---------------------------------------------------------------------------
run:
	@if [ -z "$(E)" ]; then \
		echo "Usage: make run E=<example> [ARGS=\"...\"]"; \
		echo "Examples: $(EXAMPLES)"; \
		exit 1; \
	fi
	@BIN="$(BUILD_DIR)/$(E)$(EXT)"; \
	if [ ! -f "$$BIN" ] && [ -f "$(BUILD_DIR)/$(CONFIG)/$(E)$(EXT)" ]; then \
		BIN="$(BUILD_DIR)/$(CONFIG)/$(E)$(EXT)"; \
	fi; \
	if [ ! -f "$$BIN" ]; then \
		echo "Building $(E)..."; \
		$(MAKE) build || exit 1; \
		BIN="$(BUILD_DIR)/$(E)$(EXT)"; \
		if [ ! -f "$$BIN" ] && [ -f "$(BUILD_DIR)/$(CONFIG)/$(E)$(EXT)" ]; then \
			BIN="$(BUILD_DIR)/$(CONFIG)/$(E)$(EXT)"; \
		fi; \
	fi; \
	if [ ! -f "$$BIN" ]; then \
		echo "ERROR: $(E) not found after build."; \
		exit 1; \
	fi; \
	cd "$(BUILD_DIR)" && "./$(E)$(EXT)" $(ARGS)

print_info:
	@$(MAKE) run E=print_info ARGS="samples/big_buck_bunny.mp4"

decode_audio:
	@$(MAKE) run E=decode_audio ARGS="samples/big_buck_bunny.mp4"

decode_video:
	@$(MAKE) run E=decode_video ARGS="samples/big_buck_bunny.mp4"

encode_audio:
	@$(MAKE) run E=encode_audio

encode_video:
	@$(MAKE) run E=encode_video

filtering_audio:
	@$(MAKE) run E=filtering_audio

filtering_video:
	@$(MAKE) run E=filtering_video

remuxing:
	@$(MAKE) run E=remuxing

difference:
	@$(MAKE) run E=difference

demo:
	@$(MAKE) run E=demo

simple_interface_demo:
	@$(MAKE) run E=simple_interface_demo

## ---------------------------------------------------------------------------
## Tests — Catch2 from vcpkg
## ---------------------------------------------------------------------------
test:
	@if [ ! -f "$(VCPKG_BIN)" ] && [ ! -f "$(VCPKG_BIN).exe" ]; then \
		$(MAKE) setup || exit 1; \
	fi
	@TRIPLET=$$("$(VCPKG_BIN)" triplet 2>/dev/null || echo x64-linux); \
	PKG_CONFIG_PATH="$(VCPKG_INSTALLED)/$$TRIPLET/lib/pkgconfig:$(VCPKG_INSTALLED)/lib/pkgconfig:$$PKG_CONFIG_PATH" \
	cmake -B $(BUILD_DIR) \
		-DFFMPEGCPP_BUILD_TESTS=ON \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=$(CONFIG) \
		$(EXTRA) || { \
		echo "ERROR: CMake configure failed."; \
		exit 1; \
	}
	cmake --build $(BUILD_DIR) --config $(CONFIG) -j$(JOBS) --target ffmpeg-cpp-tests || exit 1
	cd $(BUILD_DIR) && ctest --output-on-failure -C $(CONFIG)

## ---------------------------------------------------------------------------
## Housekeeping
## ---------------------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR)
	@echo "Removed $(BUILD_DIR)/"

help:
	@echo "ffmpeg-cpp"
	@echo ""
	@echo "  make                    Build everything"
	@echo "  make <example>          Build (if needed) and run"
	@echo "  make test               Run unit tests"
	@echo "  make setup              Install vcpkg deps (Catch2)"
	@echo "  make clean              Remove build directory"
	@echo ""
	@echo "Examples: $(EXAMPLES)"
	@echo ""
	@echo "Env vars: VCPKG_ROOT, CONFIG, EXTRA"
