Simple Diary
============

Simple and lightweight diary app.

<a href="https://flathub.org/apps/details/com.bjareholt.johan.SimpleDiary">
  <img src="https://flathub.org/assets/badges/flathub-badge-en.svg" alt="Get it on flathub" width="20%">
</a>


### Features
- Saves entries in standard markdown
- Adding images to your entries
- Scales on desktops, laptops, tablets and phones
- Dark mode

### Dependencies
- meson
- GTK4
- gtkmdview
- md4c

### Building

First install dependencies listed above

Secondly run the following: meson build && ninja -C build

Executable will be built at build/src/simple-diary

### Screenshots

<div>
  <img width="49%" src="https://johan.bjareholt.com/img/projects/simple-diary/entry_view.png">
  <img width="49%" src="https://johan.bjareholt.com/img/projects/simple-diary/entry_edit.png">
</div>
