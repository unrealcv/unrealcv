<center>
<img width="500px" src="/images/pipeline.svg" alt="pipeline" class="center-block"/>
<p>Fig.1: Architecture of UnrealCV</p>
</center>

The focus of UnrealCV is to do IPC (Inter Process Communication) between a game and a computer vision algorithm. The communication can be summarized by Fig.1. A game created by Unreal Engine 4 will be extended by loading UnrealCV server as its module. When the game launches, UnrealCV will start a TCP server and wait for commands. Any program can use UnrealCV client code to send plain text UnrealCV command to control the scene or retrieve information. The design of UnrealCV makes it cross-platform and support multiple programming languages. The command system is designed in a modular way and can be easily extended.

<!-- The annotation is generated using the post processing effect of Unreal Engine. Buffer Visualization mode. -->
