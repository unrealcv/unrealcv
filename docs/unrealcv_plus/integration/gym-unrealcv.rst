gym-unrealcv Integration
========================

``gym-unrealcv`` provides Gym-compatible environments backed by UnrealCV. It
is maintained in the UnrealZoo ecosystem and is installed separately from the
UnrealCV plugin.

Repository
----------

https://github.com/UnrealZoo/unrealzoo-gym

Installation
------------

.. code-block:: console

    git clone https://github.com/UnrealZoo/unrealzoo-gym.git
    cd unrealzoo-gym
    pip install -e .

Install the environment dependencies required by the selected UnrealZoo task
and make sure its Unreal binary is available before creating an environment.

Quick Start
-----------

.. code-block:: python

    import gymnasium as gym
    import gym_unrealcv

    env = gym.make("UnrealTrack-track_train-ContinuousColor-v0")

    observation, info = env.reset()
    action = env.action_space.sample()
    observation, reward, terminated, truncated, info = env.step(action)

    env.close()

Environment Naming
------------------

Environment IDs generally follow this pattern::

    Unreal{Task}-{MapName}-{ActionSpace}{ObservationType}-v{Version}

Example::

    UnrealTrack-Greek_Island-ContinuousRgbd-v0

Tasks
~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Task
     - Description
   * - ``Track``
     - Object tracking.
   * - ``Navigation``
     - Point navigation.
   * - ``Rendezvous``
     - Target meeting.

Action Spaces
~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Space
     - Description
   * - ``Discrete``
     - Discretized actions.
   * - ``Continuous``
     - Continuous control.
   * - ``Mixed``
     - Mixed control with interactive actions.

Observation Types
~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Type
     - Description
   * - ``Color``
     - RGB image.
   * - ``Depth``
     - Depth map.
   * - ``Rgbd``
     - Combined RGB and depth data.
   * - ``Gray``
     - Grayscale image.
   * - ``Mask``
     - Segmentation mask.
   * - ``Pose``
     - Agent pose.

Example Maps
------------

- ``track_train``
- ``Greek_Island``
- ``ContainerYard_Day``
- ``ContainerYard_Night``
- ``SuburbNeighborhood_Day``
- ``SuburbNeighborhood_Night``
- ``Map_ChemicalPlant_1``
- ``Old_Town``
- ``MiddleEast``
- ``Demo_Roof``

Related Resources
-----------------

- UnrealZoo documentation: https://unrealzoo.github.io/
- UnrealCV: https://unrealcv.org/
- Gymnasium: https://gymnasium.farama.org/
