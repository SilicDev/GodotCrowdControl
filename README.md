# GodotCrowdControl

This plugin adds support for CrowdControl, allowing it to change how players play the game on streams.

Based on https://github.com/nathanfranke/gdextension and the Unity plugin at https://github.com/WarpWorld/crowd-control-unity-plugin

Tested against 4.1, 4.1.3 and 4.2.1

**This does not add effects to already released games**
-> I intend to look into this but atm this is not my priority


## Known Issues

Only partial C# support.
: Due to several issues in Godot (i.e. no GdExtension virtual functions) full C# support can not be provided. Moving C# to its own GdExtension may make this easier in the future.