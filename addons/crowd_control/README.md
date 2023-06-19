# GodotCrowdControl

This plugin adds support for CrowdControl, allowing it to change how players play the game on streams.

A simple TCP client connects to CrowdControl's client and parses event requests to activate effects that you can define. Simply create a named subclass of CCEffect and the custom registry will take care of the rest.
Based on the implementation for Celeste in [CCPack-PC-Celeste](https://github.com/WarpWorld/CCPack-PC-Celeste)

**This does not add effects to already released games**
-> I intend to look into this but atm this is not my priority