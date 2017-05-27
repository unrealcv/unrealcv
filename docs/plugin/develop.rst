===========
Development
===========

:author: :user:`Ized06`, :user:`qiuwch`

UnrealCV can be compiled as a plugin as shown in the :ref:`compile_plugin`, but it you want to modify the plugin source code, compiling it together with an UE4 C++ project will make debug much easier.

Create a C++ game project
=========================

UE4 has two types of game projects. Blueprint project and C++ project. We need a C++ project.

In a C++ project, the plugin code will be compiled together with the game project.

The simplest way to start is using the `playground project`_. Playground project is a simple UE4 project to show basic features or UE4 and UnrealCV, it serves as a development base and test platform for UnrealCV team.

.. _playground project: https://github.com/unrealcv/playground

Get the playground project by

.. code:: shell

    git clone --recursive https://github.com/unrealcv/playground.git

UnrealCV is a submodule of this repository. If you cloned the project without :code:`--recursive` and found the folder :file:`Plugins/unrealcv` empty. You can use :code:`git submodule init; git submodule update` to get the UnrealCV code.

Compile the C++ project
=======================
.. TODO: Link to Unreal Engine documentation

Windows

- Install visual studio.

- To generate visual studio solution, right click :file:`playground.uproject` and choose :code:`Generate Visual Studio project files`.

- Open the :file:`*.sln` solution file and build the solution. The plugin code will be compiled together with the game project.

Linux

- Compile UE4 from source code following `this official  instruction <https://wiki.unrealengine.com/Building_On_Linux>`__

- Put this to your `.bashrc` or `.zshrc`

.. code:: bash

    # Modify to a path that specified in step 1
    export UE4=/home/qiuwch/UnrealEngine/4.13
    UE4Editor() { bin=${UE4}/Engine/Binaries/Linux/UE4Editor; file=`pwd`/$1; $bin $file; }
    UE4GenerateProjectFiles() {
      bin=${UE4}/GenerateProjectFiles.sh; file=`pwd`/$1;
      $bin -project=${file} -game -engine;
    }

- Generate project file and use Makefile

.. code:: bash

    UE4GenerateProjectFiles playground.uproject
    make playgroundEditor
    # or make playgroundEditor-Linux-Debug
    UE4Editor playground.uproject

Mac

.. note::

    Need help for writing this section


Useful resources for development include:

- :doc:`The code API documentation </reference/api>`
- :doc:`UnrealCV architecture </reference/architecture>`

.. _add_new_command:

Add a new command
=================

UnrealCV provides a set of commands for accomplishing tasks and the list is growing. But it might not be sufficient for your task. If you need any functions that is missing, you can try to implement it yourself.

The benefit of implementing an UnrealCV command are:

1. You can use the communication protocol provided by UnrealCV to exchange data between your program and UE4.
2. You can share your code with other researchers, so that it can be used by others.

Here we will walk you through how to implement a command :code:`vset /object/[id]/rotation` to enable you set the rotation of an object.

:code:`FExecStatus` return the exec result of this command. The result will be returned as a text string.

Available variables for a command are :code:`GetWorld()`, :code:`GetActor()`, , :code:`GetLevel()`.

A new functions will be implemented in a CommandHandler. CommandDispatcher will use CommandHandler.
