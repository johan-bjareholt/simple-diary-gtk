use std::ffi::CStr;
use std::os::raw::c_char;
use gtk4::glib;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rs_utils_error_dialog(
    parent: *mut gtk4::ffi::GtkWidget,
    message: *const c_char,
) {
    if parent.is_null() || message.is_null() {
        eprintln!("Error: rs_utils_error_dialog called with null pointer");
        return;
    }

    let message_str = unsafe {
        CStr::from_ptr(message)
            .to_str()
            .unwrap_or("Invalid UTF-8 in error message")
    };

    let parent_widget: gtk4::Widget = unsafe {
        glib::translate::from_glib_none(parent)
    };

    crate::utils::error_dialog(&parent_widget, message_str);
}
