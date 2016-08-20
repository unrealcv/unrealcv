# Generate a random motion sequence of camera
import random
cam_seq_file = 'cam_seq.csv'

with open(cam_seq_file, 'w') as f:
    title = '%6s, %6s, %6s, %6s, %6s, %6s\n' % ('id', 'x', 'y', 'z', 'pitch', 'yaw')
    f.write(title)

    id = 0
    num_image = 30
    # a few both
    for _ in range(num_image):
        x = random.randint(-100, 100)
        y = random.randint(-100, 100)
        z = random.randint(0, 50)
        pitch = random.randint(-90, 90)
        yaw = random.randint(-90, 90)
        line = '%6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f\n' % (id, x, y, z, pitch, yaw)
        id += 1
        f.write(line)
    # Flush the rotation and location first.

    # a few rotation
    x, y, z = [float('nan')] * 3
    for _ in range(num_image):
        pitch = random.randint(-90, 90)
        yaw = random.randint(-90, 90)
        line = '%6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f\n' % (id, x, y, z, pitch, yaw)
        id += 1
        f.write(line)

    pitch, yaw = [float('nan')] * 2
    # a few position movement
    for _ in range(num_image):
        x = random.randint(-100, 100)
        y = random.randint(-100, 100)
        z = random.randint(0, 50)
        line = '%6.2f, %6.2f, %6.2f, %6.2f, %6.2f, %6.2f\n' % (id, x, y, z, pitch, yaw)
        id += 1
        f.write(line)
