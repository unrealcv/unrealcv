html:
	sphinx-build -n -b html . _build/html
	-xdg-open _build/html/index.html
	-explorer.exe _build\html\index.html
	-open _build/html/index.html

doxygen:
	doxygen Doxyfile

tutorial:
	# Start the docker container
	./env.sh start_mac
	sleep 2
	build_tutorial=True sphinx-build -n -b html . _build/html
	./env.sh stop_mac

.PHONY: doxygen tutorial

all: tutorial doxygen html

clean:
	rm -rf _build modules doxygen/html doxygen/latex doxygen/xml

rtd:
	pip install -r requirements.txt
	READTHEDOCS=True python conf.py
	READTHEDOCS=True sphinx-build -n -b html . _rtd/html
	xdg-open _build/html/index.html
