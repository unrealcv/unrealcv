Sensor Data Formats Reference
=============================

Overview
--------

This document describes the pixel data formats produced by each sensor type in the
UnrealCV sensor system. Understanding these formats is essential for correctly
interpreting captured data in downstream processing pipelines.

**Related Documents:**

- :doc:`../architecture/sensor-system` - Sensor system architecture
- :doc:`../overview` - UnrealCV Dev For UnrealZoo recording feature summary

Lit (RGB) Data
--------------

**Sensor:** ``ULitCamSensor``

**Format:** ``FColor`` (8-bit sRGB)

**Data Layout:**

.. code-block::

   Pixel[0]        Pixel[1]        Pixel[2]     ...
   +---+---+---+   +---+---+---+   +---+---+---+
   | R | G | B |   | R | G | B |   | R | G | B |
   +---+---+---+   +---+---+---+   +---+---+---+
   (Row-major order, scanline by scanline)

**Range:** 0-255 per channel (8-bit)

**Color Space:** sRGB

**Dimensions:** Width × Height × 4 bytes per pixel

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image

   # lit_0001.png is already in PNG format
   img = Image.open('lit_0001.png')
   rgb_array = np.array(img)  # Shape: (Height, Width, 3)

   # Or read raw bytes
   with open('lit_0001.png', 'rb') as f:
       data = f.read()

Depth Data
----------

**Sensor:** ``UDepthCamSensor``

**Format:** ``float32`` (32-bit float) or ``float16`` (16-bit half-float)

**Data Layout:**

.. code-block::

   Pixel[0]        Pixel[1]        Pixel[2]     ...
   +-----------+   +-----------+   +-----------+
   |  4 bytes  |   |  4 bytes  |   |  4 bytes  |
   |  (float)  |   |  (float)  |   |  (float)  |
   +-----------+   +-----------+   +-----------+
   (Row-major order, scanline by scanline)

**Depth Modes:**

**PlaneDepth (Default):**
   Distance from the camera's near plane to the pixel's depth value.
   Formula: ``Depth = distance(near_plane, pixel_world_position)``

**DistToCamCenter:**
   Euclidean distance from camera center to pixel world position.

**Range:** 0.0 to infinity (clamped by far plane)

**Units:** Unreal units (cm by default)

**Python Decoding:**

.. code-block:: python

   import numpy as np

   # Float16 PNG
   depth_png = Image.open('depth_0001.png')
   depth_array = np.array(depth_png).astype(np.float32) / 1000.0  # if stored as mm

   # Raw float32
   with open('depth_0001.raw', 'rb') as f:
       data = np.frombuffer(f.read(), dtype=np.float32)
   depth_array = data.reshape((height, width))

Normal Data
-----------

**Sensor:** ``UNormalCamSensor``

**Format:** ``FColor`` (8-bit encoded normals)

**Data Layout:**

.. code-block::

   Channel   Meaning              Range
   -------   ------------------   -----
   R         Normal X             0-255 (mapped to -1.0 to 1.0)
   G         Normal Y             0-255 (mapped to -1.0 to 1.0)
   B         Normal Z             0-255 (mapped to  0.0 to 1.0)
   A         Unused               255

**Encoding Formula:**

.. code-block::

   Color.R = (Normal.X + 1.0) * 127.5
   Color.G = (Normal.Y + 1.0) * 127.5
   Color.B = Normal.Z * 255.0
   Color.A = 255

**Decoding Formula:**

.. code-block::

   Normal.X = (Color.R / 255.0) * 2.0 - 1.0
   Normal.Y = (Color.G / 255.0) * 2.0 - 1.0
   Normal.Z = Color.B / 255.0
   Normal = normalize(Normal)

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image

   normal_img = Image.open('normal_0001.png')
   normal_array = np.array(normal_img).astype(np.float32) / 255.0

   normal_array[:, :, 0] = normal_array[:, :, 0] * 2.0 - 1.0  # X
   normal_array[:, :, 1] = normal_array[:, :, 1] * 2.0 - 1.0  # Y
   normal_array[:, :, 2] = normal_array[:, :, 2]               # Z

Optical Flow Data
-----------------

**Sensor:** ``UFlowCamSensor``

**Format:** ``FColor`` (8-bit encoded flow vectors)

**Data Layout:**

.. code-block::

   Channel   Meaning              Range           Scale
   -------   ------------------   --------------  ------
   R         Flow X (horizontal)  0-255           See below
   G         Flow Y (vertical)    0-255           See below
   B         Mask                0 or 255        Valid/Invalid
   A         Unused               255

**Encoding:**

Flow vectors are encoded with an offset to support negative values:

.. code-block::

   Color.R = flow_x * scale + 128
   Color.G = flow_y * scale + 128
   Color.B = 255 if valid else 0

**Default Scale:** 1.0 (adjustable in material)

**Range:** Approximately -128 to +127 per pixel

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image

   flow_img = Image.open('flow_0001.png')
   flow_array = np.array(flow_img).astype(np.float32)

   # Decode flow vectors
   flow_x = (flow_array[:, :, 0] - 128.0)  # Horizontal
   flow_y = (flow_array[:, :, 1] - 128.0)  # Vertical

   # Mask (valid pixels)
   valid_mask = flow_array[:, :, 2] > 127

Segmentation Mask
-----------------

**Sensor:** ``UAnnotationCamSensor``

**Format:** ``FColor`` (8-bit annotation colors)

**Data Layout:**

.. code-block::

   Channel   Meaning              Range
   -------   ------------------   -----
   R         Annotation Red       0-255
   G         Annotation Green     0-255
   B         Annotation Blue     0-255
   A         Unused               255

**Color Generation:**

Colors are generated by ``FColorGenerator`` using bit manipulation:

.. code-block::

   Index 0:  R=1,   G=2,   B=4   (0x010204)
   Index 1:  R=1,   G=4,   B=8   (0x010408)
   Index 2:  R=1,   G=8,   B=16  (0x010810)
   Index 3:  R=1,   G=16,  B=32  (0x011020)
   ...

**Color-to-ID Mapping:**

.. code-block:: cpp

   // From FObjectAnnotator::GetAnnotationColors()
   TMap<FString, FColor> Colors = FObjectAnnotator::GetAnnotationColors();

   // Iterate to get actor-color mappings
   for (const auto& Pair : Colors)
   {
      FString ActorName = Pair.Key;
      FColor Color = Pair.Value;
      int32 ID = (Color.R << 0) | (Color.G << 8) | (Color.B << 16);
   }

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image
   from collections import defaultdict

   # Get color-to-actor mapping from metadata
   with open('metadata.json', 'r') as f:
       metadata = json.load(f)

   seg_img = Image.open('seg_0001.png')
   seg_array = np.array(seg_img)

   # Create mask for specific actor
   actor_color = metadata['foreground_color']  # e.g., [1, 2, 4]
   mask = np.all(seg_array == actor_color, axis=-1)

One Object Mask
---------------

**Sensor:** ``UOneObjMaskCamSensor``

**Format:** ``FColor`` (8-bit)

**Description:**
Binary mask for a single specified actor. The actor appears white,
everything else appears black.

**Data Layout:**

.. code-block::

   Channel   Meaning              Range
   -------   ------------------   -----
   R         Mask Value          0 (background) or 255 (actor)
   G         Mask Value          0 (background) or 255 (actor)
   B         Mask Value          0 (background) or 255 (actor)
   A         Unused               255

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image

   mask_img = Image.open('one_obj_mask_0001.png')
   mask_array = np.array(mask_img)[:, :, 0]  # Single channel

   # Boolean mask
   is_foreground = mask_array > 127

One Object Lit
--------------

**Sensor:** ``UOneObjLitCamSensor``

**Format:** ``FColor`` (8-bit sRGB)

**Description:**
Alpha-only visibility data for the selected actor. Every RGB pixel is written
as ``0,0,0`` by the GPU capture-output pass; downstream postprocessing does not
rewrite the PNG. Opaque occluders suppress the selected actor's alpha
contribution, while translucent materials and Groom strands retain continuous
material-opacity or hair-coverage values at visible pixels.

The supported new implementation is fixed at native 1x resolution. Hard binary
3x3 neighborhoods receive a small spatial filter, while any neighborhood that
already contains intermediate alpha preserves its native center value. No
supersampled, native-mask, or resolved auxiliary render targets are allocated.

``unrealcv.OneObjLit.InvertAlpha`` defaults to ``1`` for compatibility with the
original output: background visibility is 255 and foreground visibility is 0.
Set it to ``0`` for conventional foreground alpha (background 0, foreground
255). The setting affects only alpha; RGB remains zero.

The legacy implementation and the explicit ``oneobjlit_legacy`` command use
the same direct GPU RGB-zeroing output pass while preserving their captured
alpha polarity. Both implementations write directly into the sensor render
target before asynchronous readback.

**Data Layout:** Same as Lit data

**Python Decoding:**

.. code-block:: python

   import numpy as np
   from PIL import Image

   lit_img = Image.open('one_obj_lit_0001.png')
   lit_array = np.array(lit_img)

   # Default compatibility mode: background visibility
   alpha = lit_array[:, :, 3]

To capture with the previous ShowOnlyList implementation, use::

   vget /camera/0/oneobjlit_legacy output.png ActorName

The legacy command renders only the selected actor, does not account for
occlusion by other actors, and preserves the old SceneColor alpha polarity
(selected foreground near 0, empty background near 255). Its RGB channels are
also zero. ``oneobjlit`` remains the supported occlusion-aware output.

Shadow Catcher
--------------

**Sensor:** ``UShadowCatcherCamSensor``

**Format:** ``FColor`` (8-bit sRGB)

**Description:**
Object composited over white background with baked shadows.
Used for matting workflows.

**Data Layout:** Same as Lit data

Stencil Mask
------------

**Sensor:** ``UStencilMaskCamSensor``

**Format:** ``FColor`` (8-bit encoded stencil values)

**Data Layout:**

.. code-block::

   Channel   Meaning              Range
   -------   ------------------   -----
   R         Stencil Value       0-255
   G         Stencil Value       0-255
   B         Stencil Value       0-255
   A         Unused               255

Metadata JSON
-------------

**File:** ``metadata.json``

**Schema:**

.. code-block:: json

   {
      "scene": {
         "scene_id": "scene_0001",
         "timestamp": "2025-01-15T10:30:00Z"
      },
      "camera": {
         "cid": "CID-FusionCamPawn-00",
         "location": {"x": 100.0, "y": 200.0, "z": 150.0},
         "rotation": {"pitch": 0, "yaw": 45, "roll": 0},
         "fov": 90.0,
         "projection": "perspective"
      },
      "objects": [
         {
            "id": "foreground_001",
            "category": "Human",
            "annotation_color": [1, 2, 4],
            "location": {"x": 0, "y": 0, "z": 0},
            "bounds": {"min": [-50, -50, 0], "max": [50, 50, 200]}
         }
      ],
      "trajectory": {
         "type": "rotate_left_45",
         "frame_count": 121,
         "fps": 30
      },
      "occlusion": {
         "ratio": 0.35,
         "occluders": ["chair_001", "table_001"]
      }
   }

Data Format Summary Table
-------------------------

+---------------------------+------------+------------+------------------+
| Sensor                    | Format     | Channels   | Typical Range    |
+===========================+============+============+==================+
| LitCamSensor              | PNG/FColor | 3 or 4     | 0-255 (sRGB)     |
+---------------------------+------------+------------+------------------+
| DepthCamSensor            | RAW/Float  | 1          | 0.0 - far_plane  |
+---------------------------+------------+------------+------------------+
| NormalCamSensor           | PNG/FColor | 3          | Encoded -1 to 1  |
+---------------------------+------------+------------+------------------+
| FlowCamSensor             | PNG/FColor | 3          | -128 to +127     |
+---------------------------+------------+------------+------------------+
| AnnotationCamSensor       | PNG/FColor | 3          | 0-255 (color ID) |
+---------------------------+------------+------------+------------------+
| OneObjMaskCamSensor       | PNG/FColor | 1          | 0 or 255         |
+---------------------------+------------+------------+------------------+
| OneObjLitCamSensor        | PNG/FColor | 4          | 0-255 (RGBA)     |
+---------------------------+------------+------------+------------------+
| ShadowCatcherCamSensor    | PNG/FColor | 3 or 4     | 0-255 (sRGB)     |
+---------------------------+------------+------------+------------------+
| StencilMaskCamSensor      | PNG/FColor | 3          | 0-255 (stencil)  |
+---------------------------+------------+------------+------------------+

File Extensions by Format
-------------------------

+---------------------------+------------+
| Extension                 | Format     |
+===========================+============+
| .png                      | 8-bit RGB  |
+---------------------------+------------+
| .hdr                      | 32-bit HDR |
+---------------------------+------------+
| .exr                      | Float EXR  |
+---------------------------+------------+
| .raw                      | Raw binary |
+---------------------------+------------+
| .json                     | Metadata   |
+---------------------------+------------+

Common Processing Patterns
--------------------------

**Pattern 1: Load All Modalities**

.. code-block:: python

   import numpy as np
   from PIL import Image

   modalities = ['lit', 'depth', 'normal', 'seg']

   data = {}
   for mod in modalities:
       if mod == 'depth':
           # Depth might be float32
           with open(f'{mod}_0001.raw', 'rb') as f:
               data[mod] = np.frombuffer(f.read(), dtype=np.float32)
               data[mod] = data[mod].reshape((480, 854))
       else:
           img = Image.open(f'{mod}_0001.png')
           data[mod] = np.array(img)

**Pattern 2: Apply Segmentation Mask**

.. code-block:: python

   # Get foreground segmentation
   seg = np.array(Image.open('seg_0001.png'))

   # Load lit image
   lit = np.array(Image.open('lit_0001.png'))

   # Create masked view
   foreground_mask = np.all(seg == foreground_color, axis=-1)
   foreground = lit.copy()
   foreground[~foreground_mask] = 0

**Pattern 3: Compute Object Masks**

.. code-block:: python

   from PIL import Image
   import numpy as np

   # Load segmentation
   seg = np.array(Image.open('seg_0001.png'))

   # Get unique colors
   unique_colors = np.unique(seg.reshape(-1, seg.shape[2]), axis=0)

   # Create masks for each object
   for i, color in enumerate(unique_colors):
       mask = np.all(seg == color, axis=-1)
       obj_mask = Image.fromarray((mask * 255).astype(np.uint8))
       obj_mask.save(f'object_{i:04d}_mask.png')

See Also
--------

- :doc:`../architecture/sensor-system` - Sensor system architecture and APIs
- :doc:`../overview` - UnrealCV Dev For UnrealZoo dataset and recording overview
