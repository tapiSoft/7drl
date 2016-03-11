all:
	make -j5 -C build
run: all
	build/7drl

PHONY: all run
