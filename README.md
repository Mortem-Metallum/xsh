Its Xsh. Its a UNIX shell, and it doesn't even have a fully working cd yet. There's not much to say about it.

Compilation:

cd build          # Meson is really picky about in-source compilation

meson setup .. && ninja -C .
