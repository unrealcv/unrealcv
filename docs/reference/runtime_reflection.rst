Runtime reflection
==================

``vreflect`` exposes Unreal reflected functions and properties through the UnrealCV command channel. It is useful
for inspection and targeted automation when a dedicated UnrealCV command does not exist.

Targets
-------

An ordinary target is an object name returned by ``vget /objects``. Prefix a class name or path with ``class:`` or
``cdo:`` to operate on its class default object::

    vreflect BP_Player_C_0 properties
    vreflect class:KismetMathLibrary functions

Commands
--------

``functions`` and ``properties`` return JSON descriptor arrays. ``get`` supports dotted paths through struct and
object properties. ``set`` uses Unreal's text import syntax::

    vreflect BP_Player_C_0 get RootComponent.RelativeLocation
    vreflect BP_Player_C_0 set CustomTimeDilation 0.5
    vreflect BP_Player_C_0 set RootComponent.RelativeLocation "(X=100,Y=200,Z=300)"

``call_json`` accepts a JSON object whose keys match reflected parameter names. Return and output parameters are
returned as a JSON object::

    vreflect BP_Player_C_0 call_json K2_GetActorLocation {}
    vreflect class:KismetMathLibrary call_json Add_IntInt {"A":2,"B":3}

Supported values
----------------

The bridge serializes common scalar, enum, name, string, text, object, struct, array, set, and map properties. JSON
arguments are converted through Unreal's property import rules. Parameter names and function names are
case-sensitive, and required input parameters must be present unless Unreal provides an editor-visible default.

Limits and security
-------------------

Reflection is not a sandbox. A connected client can inspect runtime state, mutate writable properties, and invoke
reflected functions with their normal side effects. Bind UnrealCV only to trusted interfaces and clients when this
feature is enabled.

Not every Unreal type has a lossless JSON representation, nested access only traverses struct and object properties,
and class default object changes affect defaults rather than an existing actor instance. Prefer a dedicated command
for stable production workflows that require validation, authorization, or a long-lived compatibility contract.
