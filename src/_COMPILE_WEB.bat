emcc -o raylib_game.html raylib_game.c game.c -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -DNDEBUG -O3 -I. -I C:/dev/raylib/GameJam/2024_OCT/raylib/src -I C:/dev/raylib/GameJam/2024_OCT/raylib/src/external -L. -L C:/dev/raylib/GameJam/2024_OCT/raylib/src -s USE_GLFW=3 -s ASYNCIFY -s ASYNCIFY_STACK_SIZE=1048576 -s TOTAL_MEMORY=128MB -s STACK_SIZE=1MB -s FORCE_FILESYSTEM=1 --preload-file resources --shell-file C:/dev/raylib/GameJam/2024_OCT/raylib/src/shell.html C:/dev/raylib/GameJam/2024_OCT/raylib/src/web/libraylib.a -DPLATFORM_WEB -DRELEASE -s EXPORTED_FUNCTIONS=["_free","_malloc","_main"] -s EXPORTED_RUNTIME_METHODS=ccall