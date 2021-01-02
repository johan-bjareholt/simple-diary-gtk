Simple Diary
============

Simple and lightweight diary app.

Many features are still missing as this is a work in progress.

### Features
- Saves entries in markdown
- Adding images to your entries
- Works for small form factor devices
- Flatpak support

### Dependencies
- GTK+3
- webkit2gtk-4.0
- md4c and md4c-html

### Building

##### Meson

First install dependencies listed above

Secondly run the following: meson build && ninja -C build

Executable will be built at build/src/simple-diary

##### Flatpak

Build the org.johanbjare.SimpleDiary.yml manifest as with any other flatpak
manifest with flatpak-builder
