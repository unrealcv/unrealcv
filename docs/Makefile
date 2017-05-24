html:
	-doxygen Doxyfile
	# The program will continue even doxygen is not installed
	sphinx-build -n -b html . _build/html
	xdg-open _build/html/index.html

clean: # Clear artifacts
	rm -rf _build modules tutorials/*_tutorial.* tutorials/*.zip doxygen/html doxygen/latex doxygen/xml

rtd:
	pip install -r requirements.txt
	READTHEDOCS=True python conf.py
	READTHEDOCS=True sphinx-build -n -b html . _rtd/html
	xdg-open _build/html/index.html
