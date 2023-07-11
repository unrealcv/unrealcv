# Python Client for UnrealCV

This is a python client for [UnrealCV](http://unrealcv.github.io), which is an open source project for [UE4](https://www.unrealengine.com/).

## Installation
``pip install unrealcv``

## Usage
Launch the game with UnrealCV plugin, then use the following code to connect to the game in Python.
````
from unrealcv import client
client.connect() # Connect to the game
if not client.isconnected(): # Check if the connection is successfully established
    print('UnrealCV server is not running. Run the game from http://unrealcv.github.io first.')
else:
    filename = client.request('vget /camera/0/lit')
``