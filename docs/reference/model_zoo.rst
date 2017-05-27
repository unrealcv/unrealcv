Model Zoo
=========

We provide compiled virtual worlds to play with. All the digital contents belong to the original author. If the program crash for any reason, you can report `an issue <https://github.com/unrealcv/unrealcv/issues>`__.

..
    TODO: add more formal license information
    The community maintained games will be hosted in the [github wiki page](http://).

Run the downloaded binary

- In Mac: Run :code:`[ProjectName].app`
- In Linux: Run :code:`./[ProjectName]/Binaries/Linux/[ProjectName]`
- In Windows: Run :code:`[ProjectName]/[ProjectName].exe`

.. _rr:

`RealisticRendering`_
---------------------

.. _RealisticRendering: https://docs.unrealengine.com/latest/INT/Resources/Showcases/RealisticRendering/

:Preview:

.. image:: ../images/realistic_rendering.png

:Description:

Realistic Rendering is a demo created by Epic Games to show the rendering power of Unreal Engine 4. Here we provide an extended version of this demo with UnrealCV embedded.

:Download:

- `Windows <http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries//RealisticRendering-Win64-65d6144-171cd97.zip>`__

- `Linux <http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries//RealisticRendering-Linux-65d6144-171cd97.zip>`__

(Tested in Ubuntu 14.04)


- `Mac`_

.. _mac: http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries//RealisticRendering-Mac-65d6144-c25660b.zip

- `Docker`

.. code:: bash

    `nvidia-docker run -it --rm --env="DISPLAY=:0.0" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" -p 9000:9000 -v /home/qiuwch/workspace/unrealcv-develop-branch/test/output:/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output qiuwch/rr /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering`

`First Person Shooter`_
-----------------------

:doc:`Guide to submit a binary </plugin/package>`

.. TODO: Under construction
