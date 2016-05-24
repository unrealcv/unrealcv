# Config of faster rcnn
from test_common import *
import os, sys

# RCNN config
rcnn_path = '/home/qiuwch/workspace/py-faster-rcnn'
sys.path.append(os.path.join(rcnn_path, 'tools'))
import demo as D # Use demo.py provided in faster-rcnn
import numpy as np
import matplotlib.pyplot as plt
net = None

def init_caffe(): # TODO: parse args into here
    global net
    prototxt = os.path.join(D.cfg.MODELS_DIR, D.NETS['vgg16'][0],
    'faster_rcnn_alt_opt', 'faster_rcnn_test.pt')
    caffemodel = os.path.join(D.cfg.DATA_DIR, 'faster_rcnn_models'
    , D.NETS['vgg16'][1])
    gpu_id = 0
    D.caffe.set_mode_gpu()
    D.caffe.set_device(gpu_id)
    D.cfg.GPU_ID = gpu_id
    D.cfg.TEST.HAS_RPN = True
    net = D.caffe.Net(prototxt, caffemodel, D.caffe.TEST)

    # Warmup on a dummy image
    im = 128 * np.ones((300, 500, 3), dtype = np.uint8)
    for i in xrange(2):
        _, _ = D.im_detect(net, im)

def plot_image(image, boxes, scores):
    ax.cla() # Clear axis
    ax.imshow(image, aspect='equal')

    CONF_THRESH = 0.8
    NMS_THRESH = 0.3
    for cls_ind, cls in enumerate(D.CLASSES[1:]):
        cls_ind += 1 # Skip background
        cls_boxes = boxes[:, 4*cls_ind:4*(cls_ind+1)]
        cls_scores = scores[:, cls_ind]
        dets = np.hstack((cls_boxes, cls_scores[:,np.newaxis])).astype(np.float32)
        keep = D.nms(dets, NMS_THRESH)
        dets = dets[keep, :]
        plot_bb(image, cls, dets, thresh=CONF_THRESH)

    fig.canvas.draw()

def plot_bb(im, class_name, dets, thresh=0.5):
    inds = np.where(dets[:, -1] >= thresh)[0] #
    if len(inds) == 0:
        return

    for i in inds:
        bbox = dets[i, :4]
        score = dets[i, -1]

        patch = plt.Rectangle((bbox[0], bbox[1]), bbox[2] - bbox[0]
        , bbox[3] - bbox[1], fill=False, edgecolor='red', linewidth=3.5)
        ax.add_patch(patch)
        text = '{:s} {:.3f}'.format(class_name, score)
        ax.text(bbox[0], bbox[1] - 2, text, bbox=dict(facecolor='blue', alpha=0.5)
        , fontsize=14, color='white')

def process_image(filename):
    print 'Process image: %s' % filename

    if not os.path.isfile(filename):
        print 'Image file %s not exist' % filename
        return

    image = D.cv2.imread(filename)
    timer = D.Timer()
    timer.tic()
    scores, boxes = D.im_detect(net, image)
    timer.toc()
    print ('Detection took {:.3f}s for '
    '{:d} object proposals').format(timer.total_time, boxes.shape[0])

    show_img = image[:,:, (2,1,0)] # Reorder to RGB
    plot_image(show_img, boxes, scores)


def message_handler(message):
    # This is a different thread
    filename = message
    if not net:
        init_caffe() # Caffe needs to be started in this thread, otherwise GIL will make it very slow

    print repr(message)
    lines = message.split('\n')
    if lines:
        process_image(lines[0])

if __name__ == '__main__':
    image = np.zeros((300, 300))

    client = Client((HOST, PORT), message_handler)

    # Initialize the matplotlib
    fig, ax = plt.subplots()
    ax.imshow(image)
    plt.axis('off')
    plt.tight_layout()
    plt.show()

    # client.wait() # Wait forever
