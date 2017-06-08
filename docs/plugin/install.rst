Install UnrealCV Plugin
=======================

This page briefly describes how to install UnrealCV as a UE4 plugin. Make sure you read :doc:`getting started </tutorials/getting_started>` before trying to use the plugin.

Use compiled plugin binary
--------------------------

You can download compiled UnrealCV binaries from our `github release page`_. Then copy the compiled binaries to the plugins folder to install it. Build it yourself by following the :ref:`compile_plugin`. You can install the plugin to either a game project or to UE4 engine.

- Install to project
    - Go to project folder which contains :file:`[ProjectName].uproject`
    - Create a folder called ``Plugins``
    - Put ``UnrealCV`` folder into the ``Plugins`` folder.
- Install to Unreal Engine
    - Go to the plugin folder of Unreal Engine which is ``Engine/Plugins``
    - Put ``UnrealCV`` folder into the ``Plugins`` folder.
.. note::

    If you choose to install to Unreal Engine, please make sure the version of the Unreal Engine is identical to the version of the compiled binaries downloaded.

Open ``Menu -> Edit -> Plugins``, make sure UnrealCV is installed and enabled. You have to be in play mode before you type the commands.

.. image:: ../images/plugin.png

Install from UE4 marketplace (coming)
-------------------------------------

For Windows and Mac user, UnrealCV will be released to the UE4 marketplace. We are still finalizing the submission to the UE4 marketplace and it will be available soon.

.. _compile_plugin:

Compile from source code
------------------------

If you want to try a version of UnrealCV not provided in our `github release page`_, for example, you want to try some experimental features not released yet. Compiling the plugin code from source code is the only choice.

To compile UnrealCV plugin, use :code:`build.sh` for linux and mac, use :code:`build.bat` for windows, remember to set the path of Unreal Engine by following instructions of the script. After running this command you should see ``Automation.Execute: BUILD SUCCESSFUL`` and the plugin binaries will be produced in the ``Plugins/UnrealCV`` folder. Then you can copy the compiled plugin to a UE4 project.

.. note::
    It will take some time to run this command. For windows, we suggest using ``cmd`` to run `build.bat` to make sure the path is set correctly.

If you want to modify UnrealCV code and add new features. Please refer to the :doc:`development setup </plugin/develop>`. Setting up a dev environment takes a bit more time but will make it much easier to debug and modify code.

.. note::

    When using the plugin in the editor, it is strongly recommend to turn off the setting ``Editor Preference -> General -> Misc. -> Use Less CPU when in Background``.

Special tips for Linux
----------------------

In Linux, the Unreal Engine needs to be built from source code. How to compile from source code can be found in this official document `Building On Linux`_. But the Linux version currently does not contain OpenEXR support, which is required for getting accurate depth.

To solve this, download our `OpenEXR patch for linux`_ and run :code:`git apply 0001-Fix-openexr-support-for-linux-version.patch` after running :code:`./GenerateProjectFiles.sh`. This dependency will be removed later.

.. TODO: remove openexr dependency


.. TODO: link project to their original places, release binaries with test.

.. _github release page: https://github.com/unrealcv/unrealcv/releases
.. _Building On Linux: https://wiki.unrealengine.com/Building_On_Linux
.. _OpenEXR patch for linux: https://unrealcv.github.io/files/0001-Fix-openexr-support-for-linux-version.patch
