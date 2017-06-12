html:
	sphinx-build -a -n -b html -D plot_gallery=0 . _build/html
	-xdg-open _build/html/index.html

doxygen:
	doxygen Doxyfile

tutorial:
	# Start the docker container
	./env.sh start_docker
	build_tutorial=True sphinx-build -n -b html . _build/html
	./env.sh stop_docker
	-xdg-open _build/html/index.html

test_tutorial:
	./env.sh start_docker
	cd tutorials_source && python generate_images_tutorial.py
	./env.sh stop_docker

.PHONY: doxygen tutorial

all: tutorial doxygen html

clean:
	rm -rf _build _rtd modules doxygen/html doxygen/latex doxygen/xml tutorials

rtd:
	pip install -r requirements.txt
	READTHEDOCS=True python conf.py
	READTHEDOCS=True sphinx-build -n -b html . _rtd/html
	xdg-open _build/html/index.html
