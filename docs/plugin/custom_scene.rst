Basic usage in a new project
============================

:ue4: 4.14
:unrealcv: v0.3.9
:os: mac
:level: basic
:updated: 06/27/2017

This tutorial assumes the basic knowledge of using a command line. If you are a windows user and do not have much knowledge of using command line and find it difficult to understand, please let us know.

1. Install the plugin (build from source code in this tutorial)
---------------------------------------------------------------

In this tutorial the project and plugin folders are refered as the console varialbe :code:`plugin_folder` and :code:`project_folder`

:code:`export plugin_folder=$HOME/workspace/unrealcv`

Clone the UnrealCV repository

:code:`git clone https://github.com/unrealcv/unrealcv ${plugin_folder}`

Switch to version `v0.3.9`

:code:`cd {plugin_folder}`

:code:`git checkout v0.3.9`

Build the plugin binary

:code:`export UE4="/Users/Shared/Epic Games/UE_4.14/"`

:code:`./build.sh`

More details about the plugin installation can be found in :doc:`/plugin/install`.

2. Create a new project and copy the plugin to the project folder
-----------------------------------------------------------------

Creat a 'Blueprint - First Person' project.

.. image:: http://i.imgur.com/pAbXXXi.png

Refer the project folder with a `project_folder` variable

:code:`export project_folder="$HOME/Documents/Unreal Projects/MyProject"`

Copy the compiled plugin to the project folder to install it.

:code:`cp -r "${plugin_folder}"/Plugins/ "${project_folder}"/Plugins/`

Restart the UE4 project and make sure the plugin is successfully loaded

3. Open the Unreal project and make sure the plugin is installed
----------------------------------------------------------------

:code:`cd ${project_folder}`

:code:`open MyProject.uproject`

.. image:: http://i.imgur.com/hAWJHqt.png

4. Try the command in UE4Editor
-------------------------------

.. image:: http://i.imgur.com/AcONETx.png

5. Try the command is the python console
----------------------------------------

.. image:: http://i.imgur.com/er986gI.png
