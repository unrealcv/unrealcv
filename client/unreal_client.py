# Start a client waiting to connect to server and send command to server.
import socket as S
import threading as TD

server_ip = '127.0.0.1'
port = 9000 # Read from argument
num_attempt = 1
interval = 1

# For Image loading
import matplotlib.pyplot as plt
import PIL
import numpy as np
import os
data = None
fig = None
ax = None

# For Caffe demo
import sys
sys.path.append('tools')
import demo as D
net = None

# For main
import argparse

def check_server():
    global num_attempt # without global declare, read is fine, but writing will cause undefined error, strange.
    print 'Checking server: %s:%d, count: %d' % (server_ip, port, num_attempt)

    try:
        connect_server()
    # except Exception as e:
    except S.error as e:
        print 'Server is offline'
        print e

    num_attempt += 1
    t = TD.Timer(interval, check_server)
    t.start()


def plot_rect(im, class_name, dets, thresh=0.5):
    """Draw detected bounding boxes."""
    inds = np.where(dets[:, -1] >= thresh)[0]
    if len(inds) == 0:
        return

    im = im[:, :, (2, 1, 0)]
    for i in inds:
        bbox = dets[i, :4]
        score = dets[i, -1]

        ax.add_patch(
            plt.Rectangle((bbox[0], bbox[1]),
                          bbox[2] - bbox[0],
                          bbox[3] - bbox[1], fill=False,
                          edgecolor='red', linewidth=3.5)
            )
        ax.text(bbox[0], bbox[1] - 2,
                '{:s} {:.3f}'.format(class_name, score),
                bbox=dict(facecolor='blue', alpha=0.5),
                fontsize=14, color='white')

    # ax.set_title(('{} detections with '
    #               'p({} | box) >= {:.1f}').format(class_name, class_name,
    #                                               thresh),
    #               fontsize=14)
    # plt.axis('off')
    # plt.tight_layout()
    # plt.draw()

def process_image(filename):
    print 'Process image: ', filename
    #image = PIL.Image.open(filename)
    if not os.path.isfile(filename):
        print 'Image file %s not exist' % filename
        return

    image = D.cv2.imread(filename)
    #data.set_data(image)
    show_img = image[:, :, (2, 1, 0)]
    ax.cla()
    ax.imshow(show_img, aspect='equal')

    print 'Start detection'
    timer = D.Timer()
    timer.tic()
    scores, boxes = D.im_detect(net, image)
    timer.toc()
    print ('Detection took {:.3f}s for '
           '{:d} object proposals').format(timer.total_time, boxes.shape[0])


    print 'Plot result.'
    CONF_THRESH = 0.8
    NMS_THRESH = 0.3
    for cls_ind, cls in enumerate(D.CLASSES[1:]):
        cls_ind += 1 # because we skipped background
        cls_boxes = boxes[:, 4*cls_ind:4*(cls_ind + 1)]
        cls_scores = scores[:, cls_ind]
        dets = np.hstack((cls_boxes,
                          cls_scores[:, np.newaxis])).astype(np.float32)
        keep = D.nms(dets, NMS_THRESH)
        dets = dets[keep, :]
        plot_rect(image, cls, dets, thresh=CONF_THRESH)

    fig.canvas.draw()

imgindex = 1
def connect_server():
    global imgindex
    endpoint = (server_ip, port)
    print 'Try to connect,', endpoint
    client_socket = S.socket(S.AF_INET, S.SOCK_STREAM)
    client_socket.connect(endpoint)
    print 'Connected'
    client_socket.sendall('Hello World')
    print 'Waiting Data from Server'
    init_caffe()
    while 1:
        data = client_socket.recv(1024) # Do I need a callback to receive data or constantly waiting for data?
        if not data:
            break
        filename = repr(data)
        filename = filename.strip("'")
        if filename == 'n':
            filename = '/home/qiuwch/workspace/UnrealEngine/Engine/Binaries/Linux/%04d.png' % imgindex
            imgindex += 1
        else:
            imgfolder = '/home/qiuwch/workspace/UnrealEngine/Engine/Binaries/Linux'
            filename = os.path.join(imgfolder, filename)
            print filename
        process_image(filename)
    # Keep receiving data, add a callback.
    client_socket.close()
    print 'The server socket is closed.'


def test_thread():
    filename = '/home/qiuwch/workspace/UnrealEngine/Engine/Binaries/Linux/0001.png'
    init_caffe()
    print 'Finish Init Caffe'
    process_image(filename)
    process_image(filename)

def init_caffe():
    global net
    # Initialize caffe
    prototxt = os.path.join(D.cfg.MODELS_DIR, D.NETS['vgg16'][0],
                            'faster_rcnn_alt_opt', 'faster_rcnn_test.pt')
    caffemodel = os.path.join(D.cfg.DATA_DIR, 'faster_rcnn_models',
                              D.NETS['vgg16'][1])

    gpu_id = 0
    D.caffe.set_mode_gpu()
    D.caffe.set_device(gpu_id)
    D.cfg.GPU_ID = gpu_id
    D.cfg.TEST.HAS_RPN = True  # Use RPN for proposals
    net = D.caffe.Net(prototxt, caffemodel, D.caffe.TEST)
    # Warmup on a dummy image
    im = 128 * np.ones((300, 500, 3), dtype=np.uint8)
    for i in xrange(2):
        _, _= D.im_detect(net, im)

run_server = 1
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', default=9000, type=int)
    args = parser.parse_args()
    port = args.port

    t = TD.Timer(interval, check_server) # This is an ugly solution, TODO: replace it later.
    if run_server:
        t.start()

    image = np.zeros((300, 300))
    fig, ax = plt.subplots()
    data = ax.imshow(image)

    filename = '/home/qiuwch/workspace/UnrealEngine/Engine/Binaries/Linux/0001.png'
    if not run_server:
        # process_image(filename)
        test_thread = TD.Timer(interval, test_thread)
        test_thread.start()

    plt.axis('off')
    plt.tight_layout()
    plt.show()
