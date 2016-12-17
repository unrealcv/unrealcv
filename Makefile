UE4Version=4.13
precompiled=unrealcv-$(shell python client/scripts/get-version.py)-$(UE4Version).zip

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

OTHER_SOURCE =        \
	client/           \
	docs/             \
	test/             \

package:
	# Package plugin to a zip file
	rm -rf built/Intermediate
	cd built && zip -r ../${precompiled} *

rebuild:
	rm -rf built/
	sh build.sh

clean:
	rm -rf Intermediate/ Binaries/ built/ *.zip

submit: clean
	# UE4 market requires one zip for each supported version
	zip -r 4.12.zip $(PLUGIN_SOURCE)
	zip -r 4.13.zip $(PLUGIN_SOURCE)
	zip -r 4.14.zip $(PLUGIN_SOURCE)
	
	
