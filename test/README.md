The development of UnrealCV is supported by a set of test to ensure the correctness.

## How to run test

After downloading a game binary or installing the plugin, it is recommended to run some basic tests first to ensure UnrealCV works correctly.

Install dependencies first, `pip install -r requirements.txt`, then run the elementary test with `pytest . -x`

If you want to run a single test, use `pytest [filename].py`, or more specifically `pytest filename::test_name`.

Other tips of pytest: `pytest -x`, the `-x` parameter will stop when encounter the first error. Use `pytest -v` if you prefer a verbose output. Show more diagnositic information, try `pytest -s .` Use ipdb module in pytest.

Make sure either 1. The game binary is started. or 2. The editor has UnrealCV installed, and is in `Play` mode.

## File Structure of the test folder

The test scripts usually ended with a suffix '\_test.py'

```
# Core test
- client/                   # Test the python client code with a dummy python socket server,
                            # No need for user to run this, it is for travis.

# Utility
- benchmark_report.py       # Show some speed benchmark of UnrealCV.
- docker_util.py            # Support automatic nvidia-docker execution.
- conftest.py               # Test configuration file for pytest.
- requirements.txt          # Requirements to run test scripts

# Test
- connection_test.py        # Make sure the client can successfully connect to the UnrealCV server, also include throughput test
-
```

## Docker

It is also possible to use docker to run the test automatically. The docker makes our CI system run smoothly without the requirement to launch game binary manually. The docker image is produced by [qiuwch/unrealcv-docker-images](https://github.com/qiuwch/unrealcv-docker-images)

Test whether the docker image can be successfully run, `python docker_util.py`.

Use `pytest --docker` if you want to run the virtual environment in docker. UnrealCV docker requires nvidia-docker, since we need the computation power of GPU. Right now only Linux can support nvidia-docker.

## Wishlist
TODO: Add tox to support test of python3.
TODO: Verify whether pytest runs tests simultaneously or serially.
