use std::ffi::CStr;
use std::os::raw::c_char;
use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{ToGlibPtr, FromGlibPtrNone, IntoGlib};
use glib::ffi::GType;

use super::entry_rename_dialog::EntryRenameDialog;

#[unsafe(no_mangle)]
pub extern "C" fn entry_rename_dialog_get_type() -> GType {
    EntryRenameDialog::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_rename_dialog_new(basename: *const c_char) -> *mut gtk4::ffi::GtkWindow {
    if basename.is_null() {
        return std::ptr::null_mut();
    }

    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let basename_str = unsafe {
        CStr::from_ptr(basename).to_str().unwrap_or("")
    };

    let dialog = EntryRenameDialog::new(basename_str);
    let window: gtk4::Window = dialog.upcast();
    window.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_rename_dialog_open(
    dialog: *mut gtk4::ffi::GtkWindow,
    parent: *mut gtk4::ffi::GtkWindow,
) {
    if dialog.is_null() || parent.is_null() {
        return;
    }

    let window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(dialog)
    };
    let dialog = window.downcast::<EntryRenameDialog>().expect("Not an EntryRenameDialog");

    let parent_window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(parent)
    };

    dialog.open(&parent_window);
}
