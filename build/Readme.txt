Prerequisites:
- Microsoft Visual Studio 2013 or later (Express/Community Edition is sufficient)
- SFML (include and lib path have to be set in Visual Studio)
- gltext.h (can be downloaded here: http://www.opengl.org/registry/api/glext.h)

To build the installer, you'll also need WiX.

Building:
Start the solution, choose configuration and press F7/Build. If you are not using Visual Studio 2015, you will need to change platform toolset in the project settings.

Configurations:
The solution provides basically 4 configurations: Debug and Release for both x64 an x86 (Win32). While the x64 is configured to build against v140 toolset, the x86 solution builds against v140_xp toolset to enable Windows XP support. All configurations link runtime dynamically, so users have to have the Visual C++ Redistributable package installed. It should get installed automatically if you are using the installer.