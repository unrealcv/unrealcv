The documentation is hosted in http://docs.unrealcv.org.

The ipython notebook will not be executed by default. If you want to rebuild the ipython notebook tutorials in tutorials_source folder. Use `make tutorials`.

Use `make` to compile the documentation locally, but we recommend you to read the online version. If you prefer a pdf version, you can download it from [here](http://readthedocs.org/projects/unrealcv/downloads/pdf/develop/).

## More technical details of the docs

IPython notebook: This documentation contains a `tutorials_source` folder, which contains python source code which will be built to an ipython notebook. Type `make` will not run the tutorial, but will use the `git-lfs` cache. Use `make tutorial` to build the ipython notebook.

The API documentation, especially for the C++ API, is written using `doxygen`. If doxygen is not installed, the `make` of documentation can still be successful. But the `api.html` page will show errors.

You can also read the documentation directly in the github by open `index.rst` file. But it will show some errors.
