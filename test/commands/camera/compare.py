import cv2, math, os
import numpy as np

with open('cam_seq.csv', 'r') as f:
    lines = f.readlines()
    lines = lines[1:]
    seqs = [[float(v) for v in line.split(',')] for line in lines]

folder1 = 'fast'
folder2 = 'slow'

if not os.path.isdir('compare'):
    os.mkdir('compare')

diffs = []
for (id, x, y, z, pitch, yaw) in seqs:
    suffix = ''
    if not (math.isnan(x) or math.isnan(y) or math.isnan(z)):
        suffix += '_loc'
    if not (math.isnan(pitch) or math.isnan(yaw)):
        suffix += '_rot'

    for folder in [folder1, folder2]:
        file1 = os.path.join(folder1, '%06d%s.png') % (id, suffix)
        file2 = os.path.join(folder2, '%06d%s.png') % (id, suffix)
        im1 = cv2.imread(file1)
        im2 = cv2.imread(file2)

        diff = abs(im1 - im2)
        cv2.imwrite(os.path.join('compare', '%06d%s.png') % (id, suffix), diff)
        diffs.append(diff.sum())
        print diff.sum()

print 'max %.2f, min %.2f, mean %.2f' % (np.max(diffs), np.min(diffs), np.mean(diffs))
