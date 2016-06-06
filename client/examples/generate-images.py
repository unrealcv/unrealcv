from ipynb_util import *

def get_image(client, mode, frame_index):
    # return client.request('vget /camera/0/%s %s' % (mode, os.path.abspath('%s.png' % mode)))
    return client.request('vget /camera/0/%s %s' % (mode, ('%d_%s.png' % (frame_index, mode))))

def render_frame(client, camera_pos, frame_index):
    pos = camera_pos[frame_index]
    loc = pos[0]; rot = pos[1] # rotation
    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok'
    files = []
    f = Namedict(
        lit = get_image(client, 'lit', frame_index),
        depth = get_image(client, 'depth', frame_index),
        object_mask = get_image(client, 'object_mask', frame_index),
        normal = get_image(client, 'normal', frame_index),
    )
    return f

def render_view(client, camera_pos, frame_index):
    pos = camera_pos[frame_index]
    loc = pos[0]; rot = pos[1] # rotation

    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vget /camera/0/view'
    return client.request(cmd)

if __name__ == '__main__':
    ue4cv.client.connect()

    camera_pos = read_camera_info('./realistic_rendering_camera_info.txt')
    # for frame_index in range(len(camera_pos)):
    #     f = render_frame(ue4cv.client, camera_pos, frame_index)
    #     print 'render images to %s' % str(f)

    ue4cv.client.request('vset /mode lit')
    files = []
    for frame_index in range(len(camera_pos)):
        f = render_view(ue4cv.client, camera_pos, frame_index)
        files.append(f)
