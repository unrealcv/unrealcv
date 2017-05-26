
'''
Author: Fangwei Zhong, Weichao Qiu
Utility scripts to start the docker image
'''
import os, re, time
import docker
import pytest

host_cmd = 'nvidia-docker run  -d -it \
    --env="DISPLAY=:0.0" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    -p 9000:9000 \
    {volume_args} \
    {image_name}'
host_cmd = re.sub('[ ]+', ' ', host_cmd)
docker_cmd = '/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering'


class DockerRunner():
    def __init__(self, image_name, volumes = None):
        '''
        image_name: the docker image to run
        volumes: volume mapping
        '''
        if volumes:
            volume_args = []
            for (k, v) in volumes:
                volume_args.append('-v %s:%s' % (k, v))
            volume_args = ' '.join(volume_args)
        else:
            volume_args = ''

        self.host_cmd = host_cmd.format(image_name = image_name, volume_args = volume_args)

    def start(self, docker_cmd = docker_cmd):
        self.docker_client = docker.from_env()
        self.check_image(image_name)

        cmd = '%s %s' % (self.host_cmd, docker_cmd)
        os.system(cmd)
        time.sleep(2)
        print('It is equivalent to run: %s' %  cmd)

    def get_ip(self):
        containers = self.docker_client.containers.list()
        return containers[0].attrs['NetworkSettings']['Networks']['bridge']['IPAddress']

    def stop(self):
        containers = self.docker_client.containers.list()
        containers[0].remove(force = True)
        time.sleep(2)

    def check_image(self, image_name):
        images = self.docker_client.images.list()

        # Check the existence of image
        found_img = False
        for i in range(len(images)):
            if images[i].tags.count(image_name) > 0:
                found_img = True
        # Download image

        if found_img == False:
            print 'Can not find images, downloading'
            self.docker_client.images.pull(image_name)
        else:
            print 'Found images'


def test_docker_runner():
    volumes = []
    runner = DockerRunner('qiuwch/rr', volumes)
    runner.start(docker_cmd)

volumes = [(os.path.abspath('output'), '/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output')]
runner = DockerRunner('qiuwch/rr', volumes)

if __name__ == '__main__':
    test_docker_runner()
