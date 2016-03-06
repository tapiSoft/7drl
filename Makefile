all:
	make -C build
run: all
	build/7drl

PHONY: all run
