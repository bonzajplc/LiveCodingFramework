# LiveCodingFramework
Live Coding Framework used at WeCan 2014 demoscene party. Githubbed for archive purposes only. Made by Michal Szymczyk (misz/plastic)

------------------------------------------

Live Coding Framework by misz/plastic

------------------------------------------

v0.91

- added FFT integration and smoothing
- added FFT history
- extra options in config file - window dimensions and always on top option

------------------------------------------

v0.9

------------------------------------------

Some basic info about the tool

It consists of two elements:
	* liveCodingPreview - opengl window showing fullscreen quad with our fancy shader
	* liveCodingGlslCompiler - custom, offline glsl compiler, that can be plugged into visual studio build system
	
First, we start liveCodingPreview. Then we open shaders/compiler/shaders.sln in visual studio. This is a solution with one project that has build chain customized with our liveCodingGlslCompiler. All files with extension *.glsl will be handled by our custom compiler. Every time coder hits compile button (or Ctrl-F7) shader will be recompiled, and if compilation succeeds, our compiler will send compiled binary to the instance of liveCodingPreview. liveCodingPreview will reload shader and we will see changes.

Try using Run.bat macro. It will start both, liveCodingPreview.exe and Visual Studio with our solution. You may need to tweak path to devenv.exe to start it (Is there any envvar showing where devenv.exe sits?)

liveCodingPreview.exe expects one input argument, path to where whole 'liveCoding tool' file structure is located. Depending on how you run (debugger, bin/liveCodingPreview.exe, bat), it may require different setting.

There are two solutions for visual studio. One for each executable. This makes debugging comunication between two easier...

There are also two optional visual studio extensions that are very useful:
 * NShader - for syntax higlighting
 * VSColorOutput - for coloring text within output window (build system errors, warnings, etc...)
 
Whole liveCodingPreview has very simple setup, based on freeglut. OpenGL 4.3 at least required. There isn't much error checking, but I don't expect anyone to have problems with debugging it.
 
If you want to add other textures, place them in liveCodingRoot/textures directory and register them in liveCoding::setupShaderInputs function. There's also liveCoding::setShaderInputs for setting custom shader parameters.
 
One word about performance. When shaders get big (and we found that people don't care about writing optimal code during compo), compile times stop beeing negliable. That's why we decided to compile "on demand" instead of compiling "while typing". User will hit Ctrl-F7 eventually to check if shader is correct. We use that moment to reload preview. liveCodingGlslCompiler creates fake opengl contex to compile shader and then sends binary to liveCodingPreview to avoid extra compilation.
 
F2 toggles between windowed/fullscreen modes. In case you have multi monitor setup, drag window over desired display and then hit F2. It will go fullscreen on that particular monitor.
 
liveCodingPreview launches with 'splashScreen' shader that displays texture file "textures/splashScreen.png".

Only 24/32-bit png images are supported.

There's a config file 'config.cfg' located in root directory that remembers window position and fullscreen state from last session. Delete the file to restore defaults. There's special variable for selecting BASS input device called 'bassRecordDevice'. Set it to 0, 1, 2 (or -1 to let BASS decide). Default value for 'bassRecordDevice' is -1 in case it's not declared in config.cfg. liveCodingPreview lists all available recording devices on startup.

To exit, press Alt-F4 or click 'x' in right-top corner of the window. ESC is ignored to prevent closing application when someone hits it accidentaly while coding.

Enjoy!
