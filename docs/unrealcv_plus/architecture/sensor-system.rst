Sensor System Architecture
==========================

Overview
--------

The Sensor System provides a unified multi-pass rendering orchestrator for capturing
ground truth data from Unreal Engine scenes. ``UFusionCamSensor`` manages 5 specialized
sensor types in a single render pass, enabling simultaneous capture of RGB, depth,
normals, optical flow, and segmentation data.

**Header:** ``Source/UnrealCV/Public/Sensor/CameraSensor/FusionCamSensor.h``

Architecture Diagram
--------------------

.. code::

   +---------------------------+
   | UFusionCamSensor         |  <-- Orchestrator Component
   +---------------------------+
   | - FusionSensors[5]       |  <-- Array of specialized sensors
   | - PreviewCamera          |
   | - MovieQualityRenderer   |
   +---------------------------+
   | LitCamSensor     | RGB    |
   | DepthCamSensor   | Depth  |
   | NormalCamSensor  | Normals|
   | AnnotationCamSensor| Seg  |
   | FlowCamSensor    | Flow   |
   +---------------------------+

   +---------------------------+
   | UBaseCameraSensor         |  <-- Base Class
   +---------------------------+
   | - SceneCaptureComponent2D |
   | - TextureRenderTarget2D   |
   | - FRHIGPUTextureReadback |
   +---------------------------+
   | - Capture()               |  <-- Sync capture
   | - CaptureFast()           |  <-- Async capture
   | - CaptureFastToFile()     |  <-- Async file write
   +---------------------------+

UFusionCamSensor
----------------

``UFusionCamSensor`` is the main orchestrator component that manages multiple specialized
sensors for ground truth capture.

**Component Hierarchy:**

.. code-block::

   UFusionCamSensor (PrimitiveComponent)
   ├── UCameraComponent PreviewCamera
   ├── UBaseCameraSensor FusionSensors[0..4]
   │   ├── ULitCamSensor
   │   ├── UDepthCamSensor
   │   ├── UAnnotationCamSensor
   │   ├── UNormalCamSensor
   │   └── UFlowCamSensor
   ├── UMovieQualityRenderComponent
   └── Additional Sensors
       ├── UAnnotationCamSensor OneObjectMaskCamSensor
       ├── ULitCamSensor OneObjectLitCamSensor
       ├── UShadowCatcherCamSensor
       └── UStencilMaskCamSensor

Sensor Enumerations
~~~~~~~~~~~~~~~~~~~

ELitMode
^^^^^^^^

.. code-block:: cpp

   UENUM(BlueprintType)
   enum class ELitMode : uint8
   {
      Lit,    // Standard lit rendering
      Slow    // Alternative slow path
   };

EDepthMode
^^^^^^^^^^

.. code-block:: cpp

   UENUM(BlueprintType)
   enum class EDepthMode : uint8
   {
      PlaneDepth,      // Distance from camera plane
      DistToCamCenter   // Distance to camera center point
   };

ESegMode
^^^^^^^^

.. code-block:: cpp

   UENUM(BlueprintType)
   enum class ESegMode : uint8
   {
      AnnotationComponent,  // Uses AnnotationComponent
      VertexColor,         // Uses vertex colors
      CustomStencil         // Uses custom stencil buffer
   };

EPresetFilmSize
^^^^^^^^^^^^^^^

.. code-block:: cpp

   UENUM(BlueprintType)
   enum class EPresetFilmSize : uint8
   {
      F640x480,   // 640x480 (480p)
      F720p,      // 1280x720
      F1080p      // 1920x1080
   };

Sensor Accessors
~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UDepthCamSensor* GetDepthCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UNormalCamSensor* GetNormalCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UAnnotationCamSensor* GetAnnotationCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   ULitCamSensor* GetLitCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UFlowCamSensor* GetFlowCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UShadowCatcherCamSensor* GetShadowCatcherCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UStencilMaskCamSensor* GetStencilMaskCamSensor() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   UMovieQualityRenderComponent* GetMovieQualityRenderer() const;

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   TArray<UBaseCameraSensor*> GetSensors() const;

Sensor Data Capture
~~~~~~~~~~~~~~~~~~~

**RGB / Lit Data:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetLit(TArray<FColor>& LitData, int& InOutWidth, int& InOutHeight,
                ELitMode LitMode = ELitMode::Lit);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveLitToFile(const FString& Filename);

**Depth Data:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetDepth(TArray<float>& DepthData, int& InOutWidth, int& InOutHeight,
                 EDepthMode DepthMode = EDepthMode::PlaneDepth);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveDepthToFile(const FString& Filename);

**Normal Data:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetNormal(TArray<FColor>& NormalData, int& Width, int& Height);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveNormalToFile(const FString& Filename);

**Optical Flow:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetFlow(TArray<FColor>& FlowData, int& Width, int& Height);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveFlowToFile(const FString& Filename);

**Segmentation:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetSeg(TArray<FColor>& ObjMaskData, int& Width, int& Height,
               ESegMode SegMode = ESegMode::AnnotationComponent);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveSegToFile(const FString& Filename);

**Object-Specific Capture:**

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetOneObjMask(AActor* Actor, TArray<FColor>& Data, int& InOutWidth, int& InOutHeight);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveOneObjMaskToFile(AActor* Actor, const FString& Filename);

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetOneObjLit(AActor* Actor, TArray<FColor>& Data, int& InOutWidth, int& InOutHeight);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SaveOneObjLitToFile(AActor* Actor, const FString& Filename);

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetShadowCatcher(AActor* Actor, TArray<FColor>& Data, int& InOutWidth, int& InOutHeight);

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetStencilMask(AActor* Actor, TArray<FColor>& Data, int& InOutWidth, int& InOutHeight);

Camera Transform
~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   FVector GetSensorLocation();

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   FRotator GetSensorRotation();

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetSensorLocation(FVector Location);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetSensorRotation(FRotator Rotator);

Film Size / Resolution
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetFilmSize(int Width, int Height);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   float GetFilmWidth();

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   float GetFilmHeight();

FOV / Projection
~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   float GetSensorFOV();

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetSensorFOV(float FOV);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetProjectionType(ECameraProjectionMode::Type ProjectionType);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetOrthoWidth(float OrthoWidth);

Exposure Settings
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetExposureMethod(EAutoExposureMethod ExposureMethod);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetExposureBias(float ExposureBias);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetAutoExposureSpeed(float ExposureSpeedDown, float ExposureSpeedUp);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetAutoExposureBrightness(float MinBrightness, float MaxBrightness);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetApplyPhysicalCameraExposure(int ApplyPhysicalCameraExposure);

Rendering Settings
~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetLitCaptureSource(ESceneCaptureSource CaptureSource);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetReflectionMethod(EReflectionMethod::Type Method);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type Method);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetMotionBlurParams(float MotionBlurAmount, float MotionBlurMax,
                            float MotionBlurPerObjectSize, int MotionBlurTargetFPS);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetFocalParams(float FocalDistance, float FocalRegion);

Lens Artifacts
~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetChromaticAberration(float Intensity);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetVignetteIntensity(float Intensity);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetFilmGrain(float Intensity, float TexelSize = 1.0f);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetConvolutionBloom(EBloomMethod Method, UTexture2D* KernelTexture,
                            float Intensity = 1.0f);

Async Capture Control
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetAsyncCaptureNextFrame(bool bEnabled);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   void SetUseFastCapture(bool bInUseAsync);

   UFUNCTION(BlueprintCallable, Category = "unrealcv")
   bool GetUseFastCapture() const;

Parameter Getters
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   EReflectionMethod::Type GetReflectionMethod() const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   EDynamicGlobalIlluminationMethod::Type GetGlobalIlluminationMethod() const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   EAutoExposureMethod GetExposureMethod() const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetAutoExposureSpeed(float& OutExposureSpeedDown, float& OutExposureSpeedUp) const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetMotionBlurParams(float& OutMotionBlurAmount, float& OutMotionBlurMax,
                            float& OutMotionBlurPerObjectSize, int& OutMotionBlurTargetFPS) const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetFocalParams(float& OutFocalDistance, float& OutFocalRegion) const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   float GetChromaticAberration() const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   float GetVignetteIntensity() const;

   UFUNCTION(BlueprintPure, Category = "unrealcv")
   void GetBloomParams(EBloomMethod& OutBloomMethod, float& OutBloomIntensity) const;

UBaseCameraSensor
-----------------

Base class for all camera sensors, providing common capture functionality.

**Header:** ``Source/UnrealCV/Public/Sensor/CameraSensor/BaseCameraSensor.h``

Sensor Transform
~~~~~~~~~~~~~~~~

.. code-block:: cpp

   FVector GetSensorLocation()
   {
      return this->GetComponentLocation();
   }

   void SetSensorLocation(FVector Location)
   {
      this->SetWorldLocation(Location);
   }

   FRotator GetSensorRotation()
   {
      return this->GetComponentRotation();
   }

   void SetSensorRotation(FRotator Rotator)
   {
      this->SetWorldRotation(Rotator);
   }

FOV / Film Size
~~~~~~~~~~~~~~~

.. code-block:: cpp

   float GetFOV() { return this->FOVAngle; }
   void SetFOV(float FOV) { this->FOVAngle = FOV; }

   void SetFilmSize(int Width, int Height);
   int GetFilmWidth();
   int GetFilmHeight();

Texture Target Management
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   virtual void InitTextureTarget(int FilmWidth, int FilmHeight);

   void SetPostProcessMaterial(TScriptInterface<IBlendableInterface> PostProcessMaterial);

   bool CheckTextureTarget();

Visibility Control
~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   void SetShowOnlyList(const TArray<TWeakObjectPtr<UPrimitiveComponent>>& InShowOnlyComponents);

   void HideActor(AActor* Actor);
   void ShowActor(AActor* Actor);

Sync Capture
~~~~~~~~~~~~

.. code-block:: cpp

   void Capture(TArray<FColor>& ImageData, int& Width, int& Height);
   void Capture(TArray<FFloat16Color>& ImageData, int& Width, int& Height);

   void ReadCaptureResults(TArray<FColor>& Data);

Async Capture
~~~~~~~~~~~~~

.. code-block:: cpp

   void SetUseFastCapture(bool bInUseFast) { bUseFastCapture = bInUseFast; }
   bool GetUseFastCapture() const { return bUseFastCapture; }
   bool bAsyncCaptureNextFrame;

   virtual void LaunchCapture();

   void CaptureFastToFile(const FString& Filename);

   void CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height);
   void CaptureFast(TArray<FFloat16Color>& ImageData, int& Width, int& Height);

   void ConfigureMaxQualityLumen();

Capture Format
~~~~~~~~~~~~~~

.. code-block:: cpp

   enum class ECaptureFormat : uint8
   {
      Invalid = 0,
      F16 = 1,      // Float16 (HDR)
      UInt8 = 2,    // 8-bit color
   };

   void CheckCaptureCache(ECaptureFormat Format);
   virtual void CopyBackCapture(ECaptureFormat Format);
   void CleanCaptureCache();

Specialized Sensors
-------------------

+---------------------------+------------------+------------------+
| Sensor Class              | Data Type        | Output Format    |
+===========================+==================+==================+
| ULitCamSensor             | RGB              | FColor           |
+---------------------------+------------------+------------------+
| UDepthCamSensor           | Depth            | float            |
+---------------------------+------------------+------------------+
| UNormalCamSensor          | Normals          | FColor           |
+---------------------------+------------------+------------------+
| UAnnotationCamSensor      | Segmentation     | FColor           |
+---------------------------+------------------+------------------+
| UFlowCamSensor            | Optical Flow     | FColor           |
+---------------------------+------------------+------------------+
| UShadowCatcherCamSensor   | Shadow Composite | FColor           |
+---------------------------+------------------+------------------+
| UStencilMaskCamSensor     | Stencil Mask     | FColor           |
+---------------------------+------------------+------------------+

Capture Pipeline
----------------

**Sync Path (Blocking):**

.. code::

   Game Thread
   ├── Validate capture request
   ├── Read pixels from GPU
   ├── Block until readback complete
   └── Return pixel data

**Async Path (Non-Blocking):**

.. code::

   Frame 1                 Frame 2                 Frame 3
   ─────────               ─────────               ─────────
   Game Thread
   ├── Enqueue render cmd
   └── Return immediately

                        Render Thread
                        ├── GPU render
                        └── Readback start

                                                Game Thread
                                                ├── Readback ready
                                                ├── PNG encode
                                                └── Async file write

Visibility Modes
----------------

The sensor system supports multiple visibility modes for layer isolation:

**Show Only List:**
   - Only specified components visible
   - Used for one-object capture modes

**Hide Actor:**
   - Hide specific actors from capture
   - Used for background-only capture

**Show Actor:**
   - Re-enable previously hidden actors

Integration with Recording
--------------------------

The sensor system integrates with ``AFusionCamCaptureActor`` for high-throughput
recording. See :doc:`../overview` for the UnrealCV Dev For `UnrealZoo <https://github.com/UnrealZoo>`_ recording feature summary.

See Also
--------

- :doc:`annotation-system` - Annotation system (used by AnnotationCamSensor)
- :doc:`../overview` - UnrealCV Dev For `UnrealZoo <https://github.com/UnrealZoo>`_ recording and camera feature summary
