import argparse, time

def commands():
    from connection_test import test_connection, test_echo
    from camera_test import test_video_stream
    cmds = dict(
        connect = test_connection,
        echo = test_echo,
        read_image = test_video_stream,
    )
    return cmds

def main():
    cmds = commands()

    parser = argparse.ArgumentParser()
    parser.add_argument('--cmd', choices=cmds.keys(), nargs='+', required=True)

    args = parser.parse_args()
    print('Benchmark commands to run %s' % ' '.join(list(args.cmd)))

    fps_counter = FPSCounter()
    for k in args.cmd:
        cmd = cmds[k]
        for i in range(1000):
            cmd(None)
            fps_counter.tick()

class FPSCounter():
    '''
    A simple FPSCounter for low frame rate, if the frame rate is extremlely high
    Then the overhead in this counter will be an issue.
    '''
    def __init__(self):
        self.reset()

    def reset(self):
        self.num_frames = 0
        self.total_time = 0
        self.total_frames = 0
        self.last_time = 0

    def tick(self):
        self.num_frames += 1
        current_time = time.time()

        if self.last_time == 0: # Initially
            self.last_time = current_time

        if current_time - self.last_time > 1:
            print 'FPS %d' % self.num_frames
            self.total_time += (current_time - self.last_time)
            self.total_frames += self.num_frames

            self.num_frames = 0
            self.last_time = current_time

    def average_fps(self):
        return float(self.total_frames) / self.total_time

if __name__ == '__main__':
    main()
