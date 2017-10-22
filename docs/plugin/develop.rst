===========
Development
===========

UnrealCV can be compiled as a plugin as shown in the :ref:`compile_plugin`, but it you want to modify the plugin source code, compiling it together with an UE4 C++ project will make debug much easier.
For development, we need to

- Create a C++ game project
- Get the corresponding plugin
- Compile the C++ project
- Add a new command


Create a C++ game project
=========================

UE4 has two types of game projects. Blueprint project and C++ project. We need a C++ project.

In a C++ project, the plugin code will be compiled together with the game project.

The simplest way to start is using the `playground project`_. Playground project is a simple UE4 project to show basic features of UE4 and UnrealCV. It serves as a development base and test platform for UnrealCV team.

.. _playground project: https://github.com/unrealcv/playground

We use git-lfs to manage large binary files. Please make sure you have installed git-lfs on your computer. Then you can get the playground project by

.. code:: shell

    git lfs clone https://github.com/unrealcv/playground.git

Get the corresponding plugin
============================

Now the folder :file:`Plugins/UnrealCV` in the repository is empty. Please refer to :doc:`/plugin/install` to get the corresponding plugin and put it in the folder.

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

- Install Xcode.

- To generate Xcode Project, right click :file:`playground.uproject` and choose :code:`Service->Generate Xcode Project`.

- Open the :file:`*.xcworkspace` file and build. The plugin code will be compiled together with the game project.


Useful resources for development, :doc:`UnrealCV architecture </reference/architecture>`

.. - :doc:`The code API documentation </reference/api>`

.. _add_new_command:

Add a new command
=================

UnrealCV provides a set of commands for accomplishing tasks and the list is growing. But it might not be sufficient for your task. If you need any function that is missing, you can try to implement it yourself.

The benefits of implementing an UnrealCV command are:

1. You can use the communication protocol provided by UnrealCV to exchange data between your program and UE4.
2. You can share your code with other researchers, so that it can be used by others.


.. note::

    You are supposed to edit your code in `playground->Plugins->UnrealCV` instead of `UE4->Plugins->UnrealCV`.


First we go through a very simple example which prints a message. Assume that we want to add a commamd :code:`vget /object/helloworld` to print "Hello World!". We need to modify two files: :file:`ObjectHandler.h` and :file:`ObjectHandler.cpp`.

In :file:`ObjectHandler.h`, we need to add a member function:

.. code:: c

    FExecStatus HelloWorld(const TArray<FString>& Args);

In :file:`ObjectHandler.cpp`, we define this member function:

.. code:: c

    FExecStatus FObjectCommandHandler::HelloWorld(const TArray<FString>& Args)
    {
	    FString Msg;
	    Msg += "Hello World!";
	    return FExecStatus::OK(Msg);
    }

Then we need to bind the command with the function:

.. code:: c

    void FObjectCommandHandler::RegisterCommands()
    {
            ...

    	    Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::HelloWorld);
	    Help = "Print Hello World";
	    CommandDispatcher->BindCommand(TEXT("vget /object/helloworld"), Cmd, Help);

            ...
    }

After the modification, we can compile and use the new command.

Here we will walk you through how to implement a command :code:`vset /object/[id]/rotation` to enable you set the rotation of an object.

:code:`FExecStatus` return the exec result of this command. The result will be returned as a text string.

Available variables for a command are :code:`GetWorld()`, :code:`GetActor()`, , :code:`GetLevel()`.

A new function will be implemented in a CommandHandler. CommandDispatcher will use CommandHandler.
