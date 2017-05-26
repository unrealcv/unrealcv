html:
	-doxygen Doxyfile
	# Start the docker container
	-nvidia-docker run -d -it --name rr --env="DISPLAY=:0.0" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" -p 9000:9000 qiuwch/rr /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering && sleep 2
	sphinx-build -n -b html . _build/html
	-docker stop rr; docker rm rr
	-xdg-open _build/html/index.html
	-explorer.exe _build\html\index.html

clean: 
	rm -rf _build modules doxygen/html doxygen/latex doxygen/xml

rtd:
	pip install -r requirements.txt
	READTHEDOCS=True python conf.py
	READTHEDOCS=True sphinx-build -n -b html . _rtd/html
	xdg-open _build/html/index.html
