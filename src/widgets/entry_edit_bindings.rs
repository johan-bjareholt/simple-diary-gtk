use std::ffi::CStr;
use std::os::raw::c_char;
use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{ToGlibPtr, FromGlibPtrNone, IntoGlib};
use glib::gobject_ffi;
use glib::ffi::GType;

use super::entry_edit::EntryEdit;

#[unsafe(no_mangle)]
pub extern "C" fn entry_edit_get_type() -> GType {
    EntryEdit::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_edit_new(entry: *mut gobject_ffi::GObject) -> *mut gtk4::ffi::GtkWidget {
    if entry.is_null() {
        return std::ptr::null_mut();
    }

    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let entry_obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = entry_obj.downcast::<crate::entry::Entry>().expect("Not an Entry");

    let edit = EntryEdit::new(&entry);
    let widget: gtk4::Widget = edit.upcast();
    widget.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_edit_on_back_pressed(widget: *mut gtk4::ffi::GtkWidget) {
    if widget.is_null() {
        return;
    }

    let widget: gtk4::Widget = unsafe { FromGlibPtrNone::from_glib_none(widget) };
    let entry_edit = widget.downcast::<EntryEdit>().expect("Not an EntryEdit");

    entry_edit.on_back_pressed();
}
