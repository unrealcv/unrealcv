clean:
	rm -rf Intermediate Binaries

all:
	python client/scripts/build-plugin.py --engine_path ${UnrealEngine} --dev
