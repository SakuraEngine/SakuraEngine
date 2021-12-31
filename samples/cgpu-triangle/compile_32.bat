@clang -c -O3 --target=wasm32 -I %~dp0../../include/ %~dp0triangle_module.wa.c -o %cd%/../triangle_module32.o
@wasm-ld --no-entry --strip-all --allow-undefined --export-all %cd%/../triangle_module32.o -o %cd%/../Debug/triangle_module32.wasm
@wasm-ld --no-entry --strip-all --allow-undefined --export-all %cd%/../triangle_module32.o -o %cd%/../Release/triangle_module32.wasm