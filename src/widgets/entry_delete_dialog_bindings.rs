use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{ToGlibPtr, FromGlibPtrNone, IntoGlib};
use glib::ffi::GType;

use super::entry_delete_dialog::EntryDeleteDialog;

#[unsafe(no_mangle)]
pub extern "C" fn entry_delete_dialog_get_type() -> GType {
    EntryDeleteDialog::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_delete_dialog_new() -> *mut gtk4::ffi::GtkWindow {
    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let dialog = EntryDeleteDialog::new();
    let window: gtk4::Window = dialog.upcast();
    window.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_delete_dialog_open(
    dialog: *mut gtk4::ffi::GtkWindow,
    parent: *mut gtk4::ffi::GtkWindow,
) {
    if dialog.is_null() || parent.is_null() {
        return;
    }

    let window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(dialog)
    };
    let dialog = window.downcast::<EntryDeleteDialog>().expect("Not an EntryDeleteDialog");

    let parent_window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(parent)
    };

    dialog.open(&parent_window);
}
