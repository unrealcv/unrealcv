=====================
Package a game binary
=====================

In some scenarios you will want to package your game projects into a binary, instead of using it in the editor, for example you want to deploy the game to a deep learning server without UE4 installed or share your game project with other researchers

This page briefly describes how to create a game binary with UnrealCV embedded. Some game binaries can be found in the :doc:`model zoo </reference/model_zoo>`

Guide to submit a binary
========================

1. Modify UE4 config file
2. Package your game project into a binary
3. Make a pull request to modify the :doc:`/reference/model_zoo`
4. We will review your pull request and merge the changes to the UnrealCV website

1. Modify an UE4 config file
----------------------------
First, you need to add a line to UE4 engine config file :code:`Engine\Config\ConsoleVariables.ini` by adding this line to the end.

.. https://answers.unrealengine.com/questions/544978/cant-change-viewmodes-in-packaged-build-not-allowe.html

.. code:: ini

    r.ForceDebugViewModes = 1

If this line is missing, UnrealCV commands of the packaged game will not work correctly.

2. Package your game project
----------------------------

UE4 makes it easy to release your project as a game binary. You can use the editor menu to package a game. Many related blog posts can be found online.

- Use UE4 Editor to package a game

.. TODO: Add a screenshot

- Use script to package a game

.. code:: python

    python build.py --UE4 {UE4 instal path} {uproject path}

For example, :code:`python build.py --UE4 "C:\Program Files\Epic Games\UE_4.16" C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene1\ArchinteriorsVol2Scene1.uproject`

3. Make a pull request
----------------------

The last step is making a pull request to modify the :doc:`model zoo page </reference/model_zoo>` and add your content. We list some information that should be provided in the pull request. These information can help others better utilize the game binary. This is :ref:`an example <rr>`.

Binary name (required):
    An easy to remember name that people can refer to the binary you created

Author information (required):
    Author name and contact information

Binary description (required):
    What this virtual world is designed for? Generating dataset, reinforcement learning, etc.??

UnrealCV version (required):
    It can be a release version such as v0.3, a git short sha version, or a pointer to a commit of your fork. This information is to help others find which API is available and the corresponding documentation.

Download link (required):
    Please host binaries in your website, if you have difficulties finding a place to host content, we can help you find some free solutions. Please also include which platform (win,mac,linux) your binaries are built for.

Related paper or project page (optional):
    If this game binary is related to a research project, make a link to here.

Packaging binaries automatically
================================

In the UnrealCV team, we use a set of packaging scripts to automate the packaging and testing. These scripts are hosted in <qiuwch/unrealcv-packaging>.
