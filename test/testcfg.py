def load_client(debug=False):
    # if DEBUG is false, use the system version, else use submodule
    if not debug:
        from unrealcv import client
    else:
        import sys
        sys.path.insert(0, './unrealcv/client/python') # overwrite system unrealcv
        from unrealcv import client
        # correctly setup logging
        import logging
        stream_handler = logging.StreamHandler()
        logger = logging.getLogger('unrealcv')
        # logger.addHandler(stream_handler)
        logger.setLevel(logging.DEBUG)

    return client

debug = False
client = load_client(debug)

if __name__ == '__main__':
    client = load_client(debug=False)
    client = load_client(debug=True)
