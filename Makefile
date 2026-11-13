PYTHON ?= python3
.PHONY: all build setup test
all: build
build:
	$(PYTHON) native/compile.py --n06
setup:
	$(PYTHON) tools/setup_native.py
test:
	$(PYTHON) tools/check_source_tree.py
	$(PYTHON) native/tests/check_n06_video.py
	$(PYTHON) native/tests/check_n061_gpu.py
