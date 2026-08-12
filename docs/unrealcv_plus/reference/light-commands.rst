Light Control Commands
======================

This section documents runtime light manipulation commands (UE5.2+).

Overview
--------

UnrealCV provides TCP commands for controlling scene lighting:

- **Directional Light**: Sunlight intensity and shadow settings
- **Sky Light**: Ambient/IBL intensity control
- **Shadow Quality**: Deep shadow casting for better occlusion

.. note::
   These commands operate on the scene's default DirectionalLight and SkyLight
   actors. Custom lights require Blueprint API usage.

TCP Commands
------------

Directional Light
~~~~~~~~~~~~~~~~~

vget /light/directional/intensity
    Get directional light intensity (lux).

    Returns: Intensity value (e.g., ``3.00``)

vset /light/directional/intensity [Intensity]
    Set directional light intensity.

    Arguments:
    - ``Intensity``: Lux value (typical range: 0.0-10.0)

    Example: ``vset /light/directional/intensity 3.0``

    Returns: ``Set DirectionalLight intensity to 3.00``

Deep Shadow
~~~~~~~~~~~

vget /light/directional/castdeepshadow
    Get directional light deep shadow casting status.

    Returns: ``true`` or ``false``

vset /light/directional/castdeepshadow [true|false]
    Enable/disable directional light deep shadow casting.

    Arguments:
    - ``true`` or ``1`` - Enable deep shadows
    - ``false`` or ``0`` - Disable deep shadows

    Example: ``vset /light/directional/castdeepshadow true``

Sky Light
~~~~~~~~~

vget /light/skylight/intensity
    Get skylight intensity.

    Returns: Intensity value (e.g., ``1.00``)

vset /light/skylight/intensity [Intensity]
    Set skylight intensity.

    Arguments:
    - ``Intensity``: Multiplier value (typical range: 0.0-3.0)

    Example: ``vset /light/skylight/intensity 1.0``

    Returns: ``Set SkyLight intensity to 1.00``

Blueprint API
-------------

LightBPLib provides Blueprint-accessible functions.

.. function:: SetDirectionalLightIntensity(WorldContext, Intensity) -> bool

   Set directional light intensity from Blueprint.

   :param WorldContext: World context object
   :param Intensity: Lux value
   :return: true if successful

.. function:: GetDirectionalLightIntensity(WorldContext) -> float

   Get directional light intensity.

   :param WorldContext: World context object
   :return: Current intensity in lux

.. function:: SetSkyLightIntensity(WorldContext, Intensity) -> bool

   Set skylight intensity from Blueprint.

   :param WorldContext: World context object
   :param Intensity: Intensity multiplier
   :return: true if successful

.. function:: GetSkyLightIntensity(WorldContext) -> float

   Get skylight intensity.

   :param WorldContext: World context object
   :return: Current intensity multiplier

.. function:: SetDirectionalLightCastDeepShadow(WorldContext, bCast) -> bool

   Enable/disable deep shadow casting.

   :param WorldContext: World context object
   :param bCast: true to enable deep shadows
   :return: true if successful

.. function:: GetDirectionalLightCastDeepShadow(WorldContext) -> bool

   Get deep shadow casting status.

   :param WorldContext: World context object
   :return: true if deep shadows enabled

Lighting Setup Notes
--------------------

Directional Light Intensity Reference:

- ``0.5`` - Dim indoor lighting
- ``1.0`` - Overcast day
- ``3.0`` - Typical daylight
- ``10.0`` - Bright direct sunlight

Sky Light Intensity Reference:

- ``0.0`` - No ambient light
- ``0.5`` - Dark interior
- ``1.0`` - Normal IBL
- ``2.0`` - Bright interior with windows

Deep Shadows:

Enable for:
- Accurate occlusion in depth/segmentation data
- Realistic shadow relationships
- Training data for depth estimation models

Disable for:
- Faster rendering (during preview)
- Situations where shadows cause ambiguity

Common Issues
-------------

**Light commands have no effect**

- Ensure scene contains DirectionalLight and SkyLight actors
- Check that lights are set as movable (not static)
- Verify light intensity values are within valid range

**Intensity values seem wrong**

- Directional light uses physical units (lux)
- Sky light uses relative multiplier
- Check UE5 lighting units settings in Project Settings

See Also
--------

- :doc:`../overview` - UnrealCV Dev For UnrealZoo rendering feature summary
- :doc:`../../reference/commands` - Base UnrealCV command reference
