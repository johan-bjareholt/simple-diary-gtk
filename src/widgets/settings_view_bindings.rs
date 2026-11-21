use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{IntoGlib, ToGlibPtr};
use glib::ffi::GType;

use super::settings_view::SettingsView;

#[unsafe(no_mangle)]
pub extern "C" fn settings_view_get_type() -> GType {
    SettingsView::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn settings_view_new() -> *mut gtk4::ffi::GtkWidget {
    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let view = SettingsView::new();
    let widget: gtk4::Widget = view.upcast();
    widget.to_glib_full()
}
