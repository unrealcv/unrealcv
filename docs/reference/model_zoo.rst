Model Zoo
=========

We provide compiled virtual worlds to play with. All the digital contents belong to the original author. If the program crash for any reason, you can report `an issue <https://github.com/unrealcv/unrealcv/issues>`__.

.. TODO: add more formal license information
    The community maintained games will be hosted in the [github wiki page](http://).

Run the downloaded binary

- In Mac: Run :code:`[ProjectName].app`
- In Linux: Run :code:`./[ProjectName]/Binaries/Linux/[ProjectName]`
- In Windows: Run :code:`[ProjectName]/[ProjectName].exe`

Read :doc:`/plugin/package`, if you are interested in sumbitting a binary to the model zoo.

.. _rr:

RealisticRendering
------------------

:Source: https://docs.unrealengine.com/latest/INT/Resources/Showcases/RealisticRendering/

:Preview:

.. image:: ../images/realistic_rendering.png

:Description: Realistic Rendering is a demo created by Epic Games to show the rendering power of Unreal Engine 4. Here we provide an extended version of this demo with UnrealCV embedded.

:Download: `Windows <http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries/RealisticRendering-Win64-65d6144-171cd97.zip>`__, `Linux <http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries/RealisticRendering-Linux-65d6144-171cd97.zip>`__ (tested in Ubuntu 14.04), `Mac <http://www.cs.jhu.edu/~qiuwch/unrealcv/binaries//RealisticRendering-Mac-65d6144-c25660b.zip>`__


- `Docker`

.. code:: bash

    `nvidia-docker run -it --rm --env="DISPLAY=:0.0" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" -p 9000:9000 -v /home/qiuwch/workspace/unrealcv-develop-branch/test/output:/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output qiuwch/rr /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering`

ArchinteriorsVol2Scene1
-----------------------

:Source: https://www.unrealengine.com/marketplace/archinteriors-vol-2-scene-01

:Preview:

.. image:: https://image.ibb.co/jb2QCa/arch1.png

:Description: ArchinteriorsVol2Scene1 is a scene of a 2 floors apartment.

:Download: `Windows <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene1-Windows-0.3.6.zip>`__, `Linux <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene1-Linux-0.3.6.zip>`__


ArchinteriorsVol2Scene2
-----------------------

:Source: https://www.unrealengine.com/marketplace/archinteriors-vol-2-scene-02

:Preview:

.. image:: https://image.ibb.co/hwmkCa/arch2.png

:Description: ArchinteriorsVol2Scene2 is a scene of a house with 1 bedroom and 1 bathroom.

:Download: `Windows <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene2-Windows-0.3.6.zip>`__, `Linux <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene2-Linux-0.3.6.zip>`__


ArchinteriorsVol2Scene3
-----------------------

:Source: https://www.unrealengine.com/marketplace/archinteriors-vol-2-scene-03

:Preview:

.. image:: https://image.ibb.co/nyC3yF/arch3.png

:Description: ArchinteriorsVol2Scene3 is a scene of an office.

:Download: `Windows <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene3-Windows-0.3.6.zip>`__, `Linux <http://cs.jhu.edu/~qiuwch/release/unrealcv/ArchinteriorsVol2Scene3-Linux-0.3.6.zip>`__


UrbanCity
---------

:Source: https://www.unrealengine.com/marketplace/urban-city

:Preview:

.. image:: https://image.ibb.co/kgrJXa/urbancity.png

:Description: UrbanCity is a scene of a block of street.

:Download: `Windows <http://cs.jhu.edu/~qiuwch/release/unrealcv/UrbanCity-Windows-0.3.6.zip>`__, `Linux <http://cs.jhu.edu/~qiuwch/release/unrealcv/UrbanCity-Linux-0.3.6.zip>`__


First Person Shooter
--------------------


.. TODO: Under construction
