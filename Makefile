# UE4 version for building plugin
UE4Version=4.13
# The precompiled binary zip file, e.g. unrealcv-v0.3.0-Win-4.13.zip
precompiled=unrealcv-$(shell python client/scripts/get-version.py)-$(UE4Version).zip

# Source code of UE4 plugin
PLUGIN_SOURCE =       \
	UnrealCV.uplugin  \
	LICENSE           \
	Makefile          \
	build.sh          \
	Source/           \
	Config/           \
	Content/          \
	Resources/        \
	# build.bat       \

# Source code of test, client, docs, etc.
OTHER_SOURCE =        \
	client/           \
	docs/             \
	test/             \

# Package plugin to a zip file
package:
	rm -rf built/Intermediate
	cd built && zip -r ../${precompiled} *

rebuild:
	rm -rf built/
	sh build.sh

clean:
	rm -rf Intermediate/ Binaries/ built/ *.zip

# UE4 market requires one zip for each supported version
submit: clean
	zip -r 4.12.zip $(PLUGIN_SOURCE)
	zip -r 4.13.zip $(PLUGIN_SOURCE)
	zip -r 4.14.zip $(PLUGIN_SOURCE)
