# webV2M

Copyright (C) 2018 Juergen Wothke

This is a JavaScript/WebAudio plugin of "V2 by farbrausch" (see https://www.kvraudio.com/product/v2-by-farbrausch) 
used to play music from .v2m files.
 
This plugin is designed to work with my generic WebAudio ScriptProcessor music player (see separate project)
but the API exposed by the lib can be used in any JavaScript program (it should look familiar to anyone 
that has ever done some sort of music player plugin). 


A live demo of this program can be found here: http://www.wothke.ch/webV2M/


## Credits

The project is based on: https://github.com/jgilje/v2m-player


## Project

All the "Web" specific additions (i.e. the whole point of this project) are contained in the 
"emscripten" subfolder. (Note: The web page contained within the "htdocs" folder is purely an example
and it is not part of this project.)

The original code from https://github.com/jgilje/v2m-player suffers 
from unaligned memory access issues. Respective issues have been fixed here (look for EMSCRIPTEN 
keyword to locate additions). Other than that jgilje's V2 source code has not been changed.


## Howto build

You'll need Emscripten (http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). The make script 
is designed for use of emscripten version 1.37.29 (unless you want to create WebAssembly output, older versions might 
also still work).

The below instructions assume that the webV2M project folder has been moved into the main emscripten 
installation folder (maybe not necessary) and that a command prompt has been opened within the 
project's "emscripten" sub-folder, and that the Emscripten environment vars have been previously 
set (run emsdk_env.bat).

The Web version is then built using the makeEmscripten.bat that can be found in this folder. The 
script will compile directly into the "emscripten/htdocs" example web folder, were it will create 
the backend_v2m.js library. (To create a clean-build you have to delete any previously built libs in the 
'built' sub-folder!) The content of the "htdocs" can be tested by first copying it into some 
document folder of a web server. 


## Depencencies

The current version requires version 1.03 (older versions will not
support WebAssembly or may have problems skipping playlist entries) 
of my https://github.com/wothke/webaudio-player.

This project comes without any music files, so you'll also have to get your own and place them
in the htdocs/music folder (you can configure them in the 'songs' list in index.html).


## License

The Artistic License 2.0 Copyright (c) 2000-2006, The Perl Foundation. (see separate LICENSE file in src folder).
