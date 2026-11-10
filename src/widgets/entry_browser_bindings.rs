use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{IntoGlib, ToGlibPtr};
use glib::ffi::GType;

use super::entry_browser::EntryBrowser;

#[unsafe(no_mangle)]
pub extern "C" fn entry_browser_get_type() -> GType {
    EntryBrowser::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_browser_new() -> *mut gtk4::ffi::GtkWidget {
    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let browser = EntryBrowser::new();
    let widget: gtk4::Widget = browser.upcast();
    widget.to_glib_full()
}
