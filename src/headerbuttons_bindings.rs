use glib::prelude::*;
use glib::translate::*;
use gtk4::prelude::*;

// Re-export the main type
pub use crate::headerbuttons::HeaderButtonsControl;

// GType getter used from C via DIARY_TYPE_HEADER_BUTTONS
#[unsafe(no_mangle)]
pub extern "C" fn header_buttons_control_get_type() -> glib::Type {
    HeaderButtonsControl::static_type()
}

// Default handlers called from C for widgets that don't implement
// the HeaderButtonsControl interface.
// Note: header_buttons_control_default_on_back_pressed is implemented
// in headerbuttons.c, so it is not exported here.

#[unsafe(no_mangle)]
pub extern "C" fn header_buttons_control_default_on_enter(
    _widget: *mut gtk4::ffi::GtkWidget,
    new_button_ptr: *mut gtk4::ffi::GtkButton,
    back_button_ptr: *mut gtk4::ffi::GtkButton,
    settings_button_ptr: *mut gtk4::ffi::GtkButton,
) {
    let new_button: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(new_button_ptr) };
    let back_button: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(back_button_ptr) };
    let settings_button: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(settings_button_ptr) };

    new_button.set_visible(false);
    back_button.set_visible(true);
    settings_button.set_visible(false);
}

#[unsafe(no_mangle)]
pub extern "C" fn header_buttons_control_default_on_leave(
    _widget: *mut gtk4::ffi::GtkWidget,
    _new_button: *mut gtk4::ffi::GtkButton,
    _back_button: *mut gtk4::ffi::GtkButton,
    _settings_button: *mut gtk4::ffi::GtkButton,
) {
}

#[unsafe(no_mangle)]
pub extern "C" fn header_buttons_control_default_on_new_pressed(_widget: *mut gtk4::ffi::GtkWidget) {}
