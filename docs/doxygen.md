UnrealCV features are implemented with native c++ and can be used in blueprint or through python API.

## Supported UE4

## Headless rendering

## Useful actors

- ADataCaptureActor


## Multiple camera system

unrealcv supports multi-modal data capture from a virtual scene.

Each modal is captured by a virtual sensor in the scene. `CameraSensor` is a type of sensors which captures 2D data. There are also other types of sensors to get 3D scene information, such as: `FSkeletonSensor`, `FVertexSensor`.

For camera sensor, the base class is `UBaseCameraSensor` which inherits from `USceneCaptureComponent2D`. `USceneCaptureComponent2D` is a component that can render the scene to a specific view.

The wrapper actor for a `SensorComponent` is `AFusionCameraActor`. The difference between actor and component. Actor is combined by different types of components to provide its functionalities. 

You can drag and drop an `AFusionCameraActor` into the scene. These actors can be controlled by C++ code or through blueprint. 

Implemented `CameraSensor` are:
- `UAnnotationCamSensor`
- `UDepthCamSensor`
- `ULitCamSensor`
- `UNormalCamSensor`


- `ULitSlowCamSensor`
- `UNontransDepthCamSensor`
- `UPlaneDepthCamSensor`
- `UStencilCamSensor`
- `UVertexColorCamSensor`
- `UVisDepthCamSensor`

- `UFusionCamSensor` is a sensor combined these functions. Users should use `UFusionCamSensor` if possible.
- `UPawnCamSensor` is a special `UFusionCamSensor` which will track the viewpoint of the Pawn automatically and this the camera 0 in the python API.

Lit, depth, normal, annotation sensors are useful. Others are not.


## Non camera sensor (legacy)

There are other non-camera sensors to support extracting other information, such as human skeleton, etc.

A partial list is
- `FSkeletonSensor`, extract data from `SkeletalMesh`
- `FVertexSensor`, extract vertex data from a mesh
- `FBoneSensor`

## Control component

- UCameraControlComponent
- ULightControlComponent
- UMaterialControlComponent

Sensor is used to extract data from the virtual scene. The input is implemented with controller.

Such as `ActorController` or `UE4CVWorldController`

# Color annotation of the scene

`UAnnotationCamSensor` extracts the rgb annotation color, the annotation color can be changed by user during runtime, or calculated when the game is started.

The color is assigned.

# Depth data
There can be two types of depth data. The distance to the camera center, or the distance to the camera plane.

# Python interface

The python API is a wrapper for the C++ core to make an AI program easy to control these C++ components.

`FExecStatus` is used to capsule the response from UnrealCV. `FCommandHandler` to handle commands.

# Server / client interface

If you want to implement a new command, please start from `FUE4CVServer`

## How unrealcv is installed into a game

- The server will start when the editor or the game launches

- AUnrealcvWorldControllerActor will be spawned into the game to annotate objects in the world and register unrealcv commands.



# Automatic injection when a game starts

The design of unrealcv makes it able to automatically inject into any existing UE4 game.

This happens as follow.

`FUnrealcvServer` starts and listen for python commands.

`FUnrealcvWorldController` makes changes to the level / map / world. 

## Use unrealcv in blueprint

Many unrealcv classes are designed to be able to use in blueprint, which means you can reuse components or use blueprint to easily extend existing features.