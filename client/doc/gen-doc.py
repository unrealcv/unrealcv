import pydoc

module = pydoc.importfile('../ue4cv.py')
pydoc.writedoc(module)
# # https://github.com/python-git/python/blob/master/Lib/pydoc.py
# object, name = pydoc.resolve(module, forceload)
# page = pydoc.html.page(describe(object), html.document(object, name))
# file = open(name + '.html', 'w')
# file.write(page)
# file.close()
# print 'wrote', name + '.html'
