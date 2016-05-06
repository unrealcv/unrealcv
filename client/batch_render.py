import ue4cv as U # Or UnrealCV ?
# I can define the sequence in here or define it in the mathinee movie editor.
def on_receive_handler(message):
    '''
    Define handler to deal with received data
    '''
    print message

class CameraConfiguration:
    def __init__(self, position, rotation):
        self.position = position
        self.rotation = rotation

def render_frame(c):
    client.send('vset /camera/0/rotation %s' % c['rotation']) # Check whether the response is successful and wait for message
    client.send('vset /camera/0/position %s' % c['position'])
    client.send('vget /camera/0/image') # Do I set filename?

if __name__ == '__main__':
    client = U.client('127.0.0.1', 9000)
    client.on_receive(on_receive_handler)
    if client.connect():
        client.send('vget /object/')
        client.send('vget')
        client.send('vget /camera/0/image')
        client.send('vget /camera/0/position')
        client.send('vset /mode/depth')

        # A list of location and rotation
        # Only valid location and rotation are acceptable
        camera_confs = []
        for _ in range(1000): # Randomly get 1000 camera configuration
            position = (0, 0, 0)
            rotation = (0, 0, 0)
            c = {'position': position, 'rotation': rotation}
            camera_confs.append(c)

        for c in camera_confs:
            render_frame(c)
