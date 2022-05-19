Simple Diary
============

Simple and lightweight diary app.

### Installation

<table>
  <tr>
    <td>Flathub</td>
    <td>
      <a href="https://flathub.org/apps/details/com.bjareholt.johan.SimpleDiary">
        <img src="https://flathub.org/assets/badges/flathub-badge-en.svg" alt="Get it on flathub" width="20%">
      </a>
    </td>
  </tr>
  <tr>
    <td>Archlinux AUR</td>
    <td>
        <a href="https://aur.archlinux.org/packages/simple-diary-gtk">simple-diary-gtk</a>,
        <a href="https://aur.archlinux.org/packages/simple-diary-gtk-git">simple-diary-gtk-git</a>
    </td>
  </tr>
</table>

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
