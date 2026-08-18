# Resources

This directory contains files shipped with Polivex.

- `i18n/` holds Qt translation sources (`.ts`). The English source catalog and every translated language live here. CMake compiles them into `.qm` files and embeds them in the application.
- `assets/` is reserved for application icons, images, sample files, and other content added later.

To add a language:

1. Copy `i18n/polivex_ru.ts` and give the copy a locale suffix, for example `polivex_de.ts`.
2. Add the file to `TS_FILES` in `src/CMakeLists.txt`.
3. Add the language to the menu in `src/ui/main_window.cpp`.
4. Run `cmake --build --preset msys2-ucrt64 --target update_translations` to collect new interface text.
5. Translate the unfinished entries, then build normally. CMake creates the `.qm` file and embeds it in the executable.
