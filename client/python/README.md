# Python Client for UnrealCV

[![PyPI version](https://badge.fury.io/py/unrealcv.svg)](https://pypi.org/project/unrealcv/)
[![Python 3.8+](https://img.shields.io/badge/python-3.8+-blue.svg)](https://www.python.org/downloads/)

This is the official Python client for [UnrealCV](https://unrealcv.org), a project to help computer vision researchers build virtual worlds using [Unreal Engine](https://www.unrealengine.com/).

## Installation

```bash
pip install unrealcv
```

## Quick Start

Launch a game with the UnrealCV plugin, then connect from Python:

```python
from unrealcv import client
client.connect()  # Connect to the game
if not client.isconnected():
    print('UnrealCV server is not running.')
else:
    # Capture an image
    filename = client.request('vget /camera/0/lit')
    print(f'Image saved to {filename}')

    # Get camera location
    location = client.request('vget /camera/0/location')
    print(f'Camera at: {location}')
```

## High-Level API

For more advanced usage (image processing, object manipulation, camera control), use the `UnrealCv_API` class:

```python
from unrealcv import UnrealCv_API

api = UnrealCv_API(port=9000, ip='127.0.0.1', resolution=(640, 480))

# Get an RGB image as a numpy array
image = api.get_image(0, 'lit')

# Get depth map
depth = api.get_depth(0)

# List all objects in the scene
objects = api.get_objects()
for obj in objects:
    location = api.get_obj_location(obj)
    print(f'{obj}: {location}')
```

> **Note:** The high-level API requires `opencv-python`, `numpy`, and `pillow`.

## Documentation

- [Full Documentation](https://docs.unrealcv.org)
- [Command Reference](https://docs.unrealcv.org/en/latest/reference/commands.html)
- [Getting Started Tutorial](https://unrealcv.github.io/tutorial/getting_started.html)

## Links

- [GitHub Repository](https://github.com/unrealcv/unrealcv)
- [UnrealZoo - Pre-built Binaries](http://unrealzoo.site/)