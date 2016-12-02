How to package a game binary.

## Guide to submit binary

The information to include should be, the UnrealCV version.

1. Package your game and produce a binary
2. Upload your binaries to a safe place
3. Make a pull request to this page

Remember to include

```
- Binary name:
- Plugin version: v0.3, if you are using a development version, include the sha version)
- Download link: The recommended filename format is
- Hash (optional): this is recommended to ensure the integrity of the binary file. UnrealCV team is not responsible for the safety of each binary and each author should be responsible for the binary they provide
- Contact information
- Citation (optional)
- Description:
```

## Use UE4 editor

## Use UnrealCV script

Jenkins system

TODO: make this script to an external place, with a lot of dependency.

```
python client/scripts/build-project.py --engine_path [EnginePath] projectfile
```
