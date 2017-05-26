The test utilize docker to run test automatically. The test script will start a docker image containing unrealcv binary automatically.


1. Install dependencies first

:code:`pip install -r requirements.txt`

2. Test whether the docker image can be successfully run.

:code:`python docker_util.py`

The binary will be started by docker by default. It is also possible to download and run the binary directly.

3. Run all python test

Test for all supported python distributions

:code:`tox .`

Run the test for your python

:code:`pytest .`

:code:`pytest --no-docker` if you want to run the virtual environment by yourself, for example in windows.

Show more diagnositic information, :code:`pytest -s .`

4. Run some specific test

:code:`pytest filename::test_name`


Files
=====
- :code:`connection_test.py`
    Whether the basic connection is successful
- :code:`benchmark_report.py`
    Report the speed
- :code:`test_config.py`

- Utility code
    - :code:`docker_util.py`


Included tests
==============
- basic connection


The development of UnrealCV is supported by a set of test to ensure the correctness. To run the test for each components, `cd` to this folder and run `py.test -x`, the `-x` parameter will stop when encounter the first error. Use `py.test -v` if you prefer a verbose output.

1. `client`

Test the correctness of UnrealCV client. The test is done with a dummy test server which mimics the function of unrealcv server.

2. `ipc`

Test the connection between the client and the plugin. Test the data exchange, throughput, framerate, etc. No unrealcv commands is involved. Make sure the plugin is stable by itself.

3. `command`

Test the correctness of unrealcv commands. If you implemented a new command, add a test to here. Organize by the hierarchical structure of the unrealcv commands.

Make sure files have been cleared before running the test.
