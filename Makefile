plugin:
	python client/scripts/build-plugin.py --engine_path ${UE4}

package: plugin
	# Package plugin to a zip file and send it to release
	
clean:
	rm -rf Intermediate Binaries
