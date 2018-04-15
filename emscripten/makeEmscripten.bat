::  POOR MAN'S DOS PROMPT BUILD SCRIPT.. make sure to delete the respective built/*.bc files before building!
::  existing *.bc files will not be recompiled. 

setlocal enabledelayedexpansion

SET ERRORLEVEL
VERIFY > NUL

:: -s PRECISE_F32=1

:: **** use the "-s WASM" switch to compile WebAssembly output. warning: the SINGLE_FILE approach does NOT currently work in Chrome 63.. ****
set "OPT= -s WASM=0 -s ASSERTIONS=1  -DNDEBUG -flto  -fno-asynchronous-unwind-tables -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections -s VERBOSE=0 -s FORCE_FILESYSTEM=1 -DRONAN -DNO_DEBUG_LOGS -DHAVE_LIMITS_H -DHAVE_STDINT_H -Wcast-align -fno-strict-aliasing -s SAFE_HEAP=0 -s DISABLE_EXCEPTION_CATCHING=0 -Wno-pointer-sign -I. -I.. -I../zlib -I../src -I../src/zlib -I../src/format -I../src/device -I../src/cpu -I../src/sdlplay -Os -O3 "
if not exist "built/v2m.bc" (
	call emcc.bat %OPT% ../src/ronan.cpp ../src/v2mplayer.cpp ../src/v2mconv.cpp ../src/synth_core.cpp ../src/sounddef.cpp -o built/v2m.bc
	IF !ERRORLEVEL! NEQ 0 goto :END
)
call emcc.bat %OPT% -s TOTAL_MEMORY=67108864 --closure 1 --llvm-lto 1  --memory-init-file 0 built/v2m.bc  adapter.cpp   -s EXPORTED_FUNCTIONS="[ '_emu_load_file','_emu_teardown','_emu_get_current_position','_emu_seek_position','_emu_get_max_position','_emu_set_subsong','_emu_get_track_info','_emu_get_sample_rate','_emu_get_audio_buffer','_emu_get_audio_buffer_length','_emu_compute_audio_samples', '_malloc', '_free']"  -o htdocs/v2m.js  -s SINGLE_FILE=0 -s EXTRA_EXPORTED_RUNTIME_METHODS="['ccall', 'Pointer_stringify']"  -s BINARYEN_ASYNC_COMPILATION=1 -s BINARYEN_TRAP_MODE='clamp' && copy /b shell-pre.js + htdocs\v2m.js + shell-post.js htdocs\web_v2m3.js && del htdocs\v2m.js && copy /b htdocs\web_v2m3.js + v2m_adapter.js htdocs\backend_v2m.js && del htdocs\web_v2m3.js
:END

