use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{from_glib_none, ToGlibPtr};
use std::ffi::CString;
use std::os::raw::c_char;

// EntryList and EntryListing are still implemented in C
#[allow(unsafe_op_in_unsafe_fn)]
unsafe extern "C" {
    fn entry_list_new() -> *mut gtk4::ffi::GtkWidget;
    fn entry_list_add_entry(list: *mut gtk4::ffi::GtkWidget, entry: *mut glib::gobject_ffi::GObject);
    fn entry_list_focus(list: *mut gtk4::ffi::GtkWidget, row: *mut gtk4::ffi::GtkListBoxRow);
    fn entry_list_unfocus(list: *mut gtk4::ffi::GtkWidget);
    fn entry_list_find(
        list: *mut gtk4::ffi::GtkWidget,
        filename: *mut c_char,
    ) -> *mut gtk4::ffi::GtkListBoxRow;
    fn entry_list_remove(
        list: *mut gtk4::ffi::GtkWidget,
        entry: *mut glib::gobject_ffi::GObject,
    ) -> glib::ffi::gboolean;
    fn entry_listing_get_entry(listing: *mut gtk4::ffi::GtkWidget) -> *mut glib::gobject_ffi::GObject;
}

fn entry_ptr(entry: &crate::entry::Entry) -> *mut glib::gobject_ffi::GObject {
    entry.upcast_ref::<glib::Object>().to_glib_none().0
}

/// Create a new entry list widget (still implemented in C)
pub fn new() -> gtk4::Widget {
    unsafe { from_glib_none(entry_list_new()) }
}

pub fn add_entry(list: &gtk4::Widget, entry: &crate::entry::Entry) {
    // The C EntryListing stores the entry as a raw pointer without taking a
    // reference, so transfer a strong reference to keep it alive.
    let entry_ptr = entry.upcast_ref::<glib::Object>().to_glib_full();
    unsafe { entry_list_add_entry(list.to_glib_none().0, entry_ptr) }
}

pub fn focus(list: &gtk4::Widget, row: &gtk4::ListBoxRow) {
    unsafe { entry_list_focus(list.to_glib_none().0, row.to_glib_none().0) }
}

pub fn unfocus(list: &gtk4::Widget) {
    unsafe { entry_list_unfocus(list.to_glib_none().0) }
}

/// Find the row for the entry with the given filename, if any
pub fn find(list: &gtk4::Widget, filename: &str) -> Option<gtk4::ListBoxRow> {
    let c_filename = CString::new(filename).expect("filename contains NUL byte");
    let row_ptr = unsafe { entry_list_find(list.to_glib_none().0, c_filename.as_ptr() as *mut c_char) };
    if row_ptr.is_null() {
        None
    } else {
        Some(unsafe { from_glib_none(row_ptr) })
    }
}

/// Remove the row for the given entry. Returns false if it wasn't in the list.
pub fn remove(list: &gtk4::Widget, entry: &crate::entry::Entry) -> bool {
    unsafe { entry_list_remove(list.to_glib_none().0, entry_ptr(entry)) != 0 }
}

/// Get the entry shown by an entry listing widget
pub fn listing_get_entry(listing: &gtk4::Widget) -> crate::entry::Entry {
    let entry_ptr = unsafe { entry_listing_get_entry(listing.to_glib_none().0) };
    let obj: glib::Object = unsafe { from_glib_none(entry_ptr) };
    obj.downcast::<crate::entry::Entry>().expect("Not an Entry")
}
