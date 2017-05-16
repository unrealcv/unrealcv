html:
	doxygen ./Doxyfile
	sphinx-build -n -b html . _build/html
	xdg-open _build/html/index.html

clean:
	rm -rf _build modules tutorials doxygen/html doxygen/latex doxygen/xml
