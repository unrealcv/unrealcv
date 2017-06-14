<<<<<<< HEAD
# Convert ipynb notebook to markdown documentation
# An alternative way is linking to ipynb files directly
#
nbconvert=jupyter nbconvert --to markdown --template ./ipynb_template.tpl

ipynb:
	${nbconvert} ../client/examples/generate-images.ipynb --output-dir ./ipynb

# Make sure no dead links in the doc.
check-docs:
	cd .. && mkdocs build && linkchecker site

.PHONY: ipynb
=======
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
>>>>>>> develop
