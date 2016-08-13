import sys
sys.path.append('../..')
import ue4cv, time
import numpy as np
from math import pi
import cv2
from math import ceil
import os, sys
import matplotlib.pyplot as plt

# Try to connect our python client to the game
#IMG_HEIGHT = 563
#IMG_WIDTH = 1207
N_img = 10
Cdist = 40.0

IMG_HEIGHT = 480.0
IMG_WIDTH = 640.0
# f = (IMG_HEIGHT**2+IMG_WIDTH**2)**(0.5) / 2
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

def D1_error(D_gt,D_est,tau):
	E = abs(D_gt-D_est)
	n_err   = ((D_gt>0)*(E>tau[0])*(E/(D_gt+1e-10)>tau[1])).sum().astype(float)
	n_total = (D_gt>0).sum()
	d_err = n_err/n_total
	return d_err

def end_point_error(D_gt, D_est):
	E = abs(D_gt-D_est)
	n_total = (D_gt>0).sum()
	E[D_gt == 0] = 0
	return E.sum() / n_total

def run_test():
	PATH = 'unrealcv-files/'
	imgL_name = 'imgL.png'
	imgR_name = 'imgR.png'
	depth_name = 'depth.exr'
	disp_gth_name = 'disp_gth.png'
	disp_est_name = 'disp.png'

	loc_l = np.asarray(ue4cv.client.request('vget /camera/0/location').split(' ')).astype(float)
	rot = np.asarray(ue4cv.client.request('vget /camera/0/rotation').split(' ')).astype(float) / 180.0 * pi
	dirct = [np.cos(rot[0])*np.cos(rot[1]), np.cos(rot[0])*np.sin(rot[1]), np.sin(rot[0])]
	left = np.cross(dirct, [0., 0., 1.])
	left = left / ((left**2).sum())**(1/2)
	loc_r = loc_l - left * Cdist
	ue4cv.client.request('vset /camera/0/location %f %f %f' % (loc_l[0], loc_l[1], loc_l[2]))
	imgL_name = ue4cv.client.request('vget /camera/0/lit ' + PATH + imgL_name)
	print 'Image is saved to %s' % imgL_name

	depth_name = ue4cv.client.request('vget /camera/0/depth ' + PATH + depth_name)
	print 'Depth is saved to %s' % depth_name

	ue4cv.client.request('vset /camera/0/location %f %f %f' % (loc_r[0], loc_r[1], loc_r[2]))
	time.sleep(1)
	imgR_name = ue4cv.client.request('vget /camera/0/lit ' + PATH + imgR_name)
	print 'Image is saved to %s' % imgR_name

	Pdepth = cv2.imread(depth_name, cv2.IMREAD_ANYCOLOR | cv2.IMREAD_ANYDEPTH)
	depth = DepthConversion(Pdepth[:,:,0])
	disp_gth = Cdist * f / depth #/ 100
	imgL = cv2.imread(imgL_name)
	imgR = cv2.imread(imgR_name)
	imgL_warp = ImageWarp(imgL, disp_gth)
	diff = abs(imgR.astype(float) - imgL_warp.astype(float)).astype(np.uint8)
	cv2.imwrite('result/' + disp_gth_name, np.uint16(disp_gth*256))
	print 'Disp GT is saved to %s' % disp_gth_name
	cv2.imwrite('result/imgL_warp.png', imgL_warp)
	print 'ImgL is saved to %s' % 'imgL_warp.png'
	cv2.imwrite('result/diff.png', diff)
	print 'Diff is save to %s' % 'diff.png'


if __name__ == '__main__':
	ue4cv.client.connect()
	run_test()
	ue4cv.client.disconnect()
