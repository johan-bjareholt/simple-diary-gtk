Simple Diary
============

Simple and lightweight diary app.

<a href="https://flathub.org/apps/details/com.bjareholt.johan.SimpleDiary">
  <img src="https://flathub.org/assets/badges/flathub-badge-en.svg" alt="Get it on flathub" width="20%">
</a>


### Features
- Saves entries in markdown
- Adding images to your entries
- Scales well on desktops, laptops, tablets and phones

### Dependencies
- meson
- GTK+3
- webkit2gtk-4.0
- md4c and md4c-html

### Building

First install dependencies listed above

Secondly run the following: meson build && ninja -C build

Executable will be built at build/src/simple-diary

### Screenshots

<div>
  <img width="32%" src="https://johan.bjareholt.com/img/projects/simple-diary/entry_list.png">
  <img width="32%" src="https://johan.bjareholt.com/img/projects/simple-diary/entry_view.png">
  <img width="32%" src="https://johan.bjareholt.com/img/projects/simple-diary/entry_edit.png">
</div>
