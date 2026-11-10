use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{FromGlibPtrNone, IntoGlib, ToGlibPtr};
use glib::ffi::GType;
use glib::gobject_ffi;

use super::entry_listing::EntryListing;

#[unsafe(no_mangle)]
pub extern "C" fn entry_listing_get_type() -> GType {
    EntryListing::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_listing_new(entry: *mut gobject_ffi::GObject) -> *mut gtk4::ffi::GtkWidget {
    if entry.is_null() {
        return std::ptr::null_mut();
    }

    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let entry_obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = entry_obj.downcast::<crate::entry::Entry>().expect("Not an Entry");

    let listing = EntryListing::new(&entry);
    let widget: gtk4::Widget = listing.upcast();
    widget.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_listing_get_entry(
    listing: *mut gtk4::ffi::GtkWidget,
) -> *mut gobject_ffi::GObject {
    if listing.is_null() {
        return std::ptr::null_mut();
    }

    let widget: gtk4::Widget = unsafe { FromGlibPtrNone::from_glib_none(listing) };
    let listing = widget.downcast::<EntryListing>().expect("Not an EntryListing");

    // Borrowed pointer, the listing keeps the entry alive
    listing.entry().upcast_ref::<glib::Object>().to_glib_none().0
}
