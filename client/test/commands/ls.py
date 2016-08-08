import sys, time
sys.path.append('../../')
import ue4cv

ue4cv.client.connect()

res = ue4cv.client.request('vget /unrealcv/help')
with open('help.txt', 'w') as f:
    f.write(res)

time.sleep(5)
