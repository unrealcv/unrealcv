# Convert ipynb notebook to markdown documentation
# An alternative way is linking to ipynb files directly
#
nbconvert=jupyter nbconvert --to markdown --template ./ipynb_template.tpl

ipynb:
	${nbconvert} ../client/examples/generate-images.ipynb --output-dir ./game

# Make sure no dead links in the doc.
check-docs:
	cd .. && mkdocs build && linkchecker site
