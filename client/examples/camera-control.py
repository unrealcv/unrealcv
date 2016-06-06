import matplotlib.pyplot as plt
import numpy as np
import ue4cv

def onpress(event):
    print event.key

if __name__ == '__main__':
    image = np.zeros((300, 300))

    fig, ax = plt.subplots()
    fig.canvas.mpl_connect('key_press_event', onpress)
    ax.imshow(image)
    plt.axis('off')
    plt.show() # Add event handler
