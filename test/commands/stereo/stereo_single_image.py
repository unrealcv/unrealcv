import time, sys, shutil
sys.path.append('../../..')
# Use timer to check time.
import ue4cv
import time, math, os, cv2
import numpy as np

Cdist = 40.0
IMG_HEIGHT = 480.0
IMG_WIDTH = 640.0
f = IMG_WIDTH / 2

def ImageWarp(imgL, disp):
	imgL_warp = np.zeros(imgL.shape).astype(np.uint8)
	for i in xrange(imgL.shape[0]):
		for j in xrange(imgL.shape[1]):
			if round(j - disp[i, j]) >= 0:
				imgL_warp[i, int(round(j - disp[i,j])), :] = imgL[i, j, :]
	return imgL_warp

def DepthConversion(PointDepth):
	H = PointDepth.shape[0]
	W = PointDepth.shape[1]
	i_c = np.float(H) / 2 - 1
	j_c = np.float(W) / 2 - 1
	columns = np.ones((H, 1)).dot(np.linspace(0, W-1, num=W).reshape((1,W)))
	rows = np.ones((W, 1)).dot(np.linspace(0, H-1, num=H).reshape((1,H))).transpose()
	DistanceFromCenter = ((rows - i_c)**2 + (columns - j_c)**2)**(0.5)
	PlaneDepth = PointDepth / (1 + (DistanceFromCenter / f)**2)**(0.5)
	return PlaneDepth

ue4cv.client.connect()

left_filename = ue4cv.client.request('vget /camera/0/lit')
right_filename = ue4cv.client.request('vget /camera/1/lit')
depth_filename = ue4cv.client.request('vget /camera/0/depth depth.exr')

# copy file
shutil.copyfile(left_filename,  'left.png')
shutil.copyfile(right_filename, 'right.png')
shutil.copyfile(depth_filename, 'depth.exr')

Pdepth = cv2.imread('depth.exr', cv2.IMREAD_ANYCOLOR | cv2.IMREAD_ANYDEPTH)
depth = DepthConversion(Pdepth[:,:,0])
disp_gth = Cdist * f / depth #/ 100

imgL = cv2.imread('left.png')
imgR = cv2.imread('right.png')
imgL_warp = ImageWarp(imgL, disp_gth)
diff = abs(imgR.astype(float) - imgL_warp.astype(float)).astype(np.uint8)
cv2.imwrite('disp_gt.png', np.uint16(disp_gth*256))
print 'Disp GT is saved to disp_gt.png'
cv2.imwrite('left_warp.png', imgL_warp)
print 'ImgL is saved to left_warp.png'
cv2.imwrite('diff.png', diff)
print 'Diff is save to diff.png'
