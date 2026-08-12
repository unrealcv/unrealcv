gym-unrealcv Integration
======================

This document covers the gym-unrealcv package for Reinforcement Learning
research with UnrealCV environments.

Overview
--------

The **gym-unrealcv** package (also called **UnrealZoo**) provides OpenAI Gym
interfaces for UnrealCV-powered environments. It enables:

- Reinforcement learning agent training
- Embodied AI research
- Navigation and tracking tasks
- Multi-agent interactions

**Repository:** https://github.com/UnrealZoo/unrealzoo-gym

Installation
-----------

.. code-block:: bash

   git clone https://github.com/UnrealZoo/unrealzoo-gym.git
   cd unrealzoo-gym
   pip install -e .

Dependencies:

- unrealcv
- gym
- opencv-python
- numpy
- matplotlib

Quick Start
----------

.. code-block:: python

   import gymnasium as gym
   import gym_unrealcv

   # Create environment
   env = gym.make('UnrealTrack-track_train-ContinuousColor-v0')

   # Reset
   obs, info = env.reset()

   # Step
   action = env.action_space.sample()
   obs, reward, terminated, truncated, info = env.step(action)

   # Close
   env.close()

Environment Naming
-----------------

Format: ``Unreal{task}-{MapName}-{ActionSpace}{ObservationType}-v{version}``

**Tasks:**

+---------------------------+----------------------------------------+
| Task                     | Description                            |
+---------------------------+----------------------------------------+
| ``Track``               | Object tracking                       |
| ``Navigation``          | Point navigation                      |
| ``Rendezvous``         | Target meeting                        |
+---------------------------+----------------------------------------+

**Action Spaces:**

+---------------------------+----------------------------------------+
| Space                    | Description                            |
+---------------------------+----------------------------------------+
| ``Discrete``             | Discretized actions                  |
| ``Continuous``           | Continuous control                    |
| ``Mixed``               | Mixed with interactive actions        |
+---------------------------+----------------------------------------+

**Observation Types:**

+---------------------------+----------------------------------------+
| Type                     | Description                            |
+---------------------------+----------------------------------------+
| ``Color``                | RGB image                             |
| ``Depth``               | Depth map                             |
| ``Rgbd``               | RGB + depth                           |
| ``Gray``                | Grayscale                             |
| ``Mask``                | Segmentation mask                     |
| ``Pose``               | Agent pose                            |
+---------------------------+----------------------------------------+

Example: ``UnrealTrack-Greek_Island-ContinuousRgbd-v0``

Available Maps
-------------

**UE4 Examples:**

- ``track_train``
- ``Greek_Island``
- ``ContainerYard_Day``
- ``ContainerYard_Night``
- ``SuburbNeighborhood_Day``
- ``SuburbNeighborhood_Night``

**UE5 Examples:**

- ``Map_ChemicalPlant_1``
- ``Old_Town``
- ``MiddleEast``
- ``Demo_Roof``

Documentation
-------------

Full documentation is available at:

- **Notion Page:** https://unrealzoo.github.io/
- **Scene Gallery:** https://www.notion.so/Scene-Gallery
- **API Docs:** See gym_unrealcv/ directory

UnrealZoo Project
-----------------

**Paper:** UnrealZoo: Enriching Photo-realistic Virtual Worlds for Embodied AI (ICCV 2025)

**Key Features:**

- 100+ photo-realistic scenes
- Diverse entities (humans, vehicles, animals)
- Multi-agent support (10+ agents)
- Interactive actions (pick/drop objects)
- Chaos system for vehicles

Citation
-------

.. code-block:: bibtex

   @misc{zhong2024unrealzooenrichingphotorealisticvirtual,
         title={UnrealZoo: Enriching Photo-realistic Virtual Worlds for Embodied AI},
         author={Fangwei Zhong and Kui Wu and Churan Wang and Hao Chen and Hai Ci and Zhoujun Li and Yizhou Wang},
         year={2024},
         eprint={2412.20977},
         archivePrefix={arXiv},
         primaryClass={cs.AI},
         url={https://arxiv.org/abs/2412.20977},
   }

Related Projects
---------------

- UnrealCV: https://unrealcv.org/
- OpenAI Gym: https://gymnasium.farama.org/
