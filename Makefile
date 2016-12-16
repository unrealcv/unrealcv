UE4Version=4.13
precompiled=unrealcv-$(shell python client/scripts/get-version.py)-$(UE4Version).zip
package:
	# Package plugin to a zip file
	rm -rf built/Intermediate
	cd built && zip -r ../${precompiled} *

rebuild:
	rm -rf built/
	sh build.sh

clean:
	rm -rf Intermediate/ Binaries/ built/
