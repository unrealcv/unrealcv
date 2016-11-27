Prerequisite: Read [development setup](plugin/dev.md) to make sure you know how to compile the plugin.

UnrealCV provides a rich set of commands for accomplishing tasks and the list is growing. But it might not be sufficient for your task. If you need any functions that is missing. You can [contact us](), or try to implement it yourself.

The benefit of implementing an UnrealCV command are:

1. You can use the communication protocol provided by UnrealCV to exchange data between your program and UE4.
2. You can share your code with other researchers, so that it can be used by others.

Here we will walk you through how to implement a command `vset /object/[id]/rotation` to enable you set the rotation of an object.

`FExecStatus` return the exec result of this command. The result will be returned as a text string.

Available variables for a command are GetWorld(); GetActor(); GetLevel();

A new functions will be implemented in a CommandHandler. CommandDispatcher will use CommandHandler.

Can I link these class definition to doxygen?
Compile html and link to an external html page, is this possible? I think so.

```
Under construction
```
