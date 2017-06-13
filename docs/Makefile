html:
	sphinx-build -a -n -b html -D plot_gallery=0 . _build/html
	-xdg-open _build/html/index.html

doxygen:
	-doxygen Doxyfile

tutorial: doxygen
	../test/docker-run.sh sphinx-build -n -b html . _build/html
	-xdg-open _build/html/index.html

test_tutorial:
	# Run the tutorial only, without populating the sphinx, for test only.
	cd tutorials_source && \
	../../test/docker-run.sh python generate_images_tutorial.py

.PHONY: doxygen tutorial

all: tutorial doxygen html

clean:
	rm -rf _build _rtd modules doxygen/html doxygen/latex doxygen/xml tutorials

rtd:
	pip install -r requirements.txt
	READTHEDOCS=True python conf.py
	READTHEDOCS=True sphinx-build -n -b html . _rtd/html
	xdg-open _build/html/index.html
