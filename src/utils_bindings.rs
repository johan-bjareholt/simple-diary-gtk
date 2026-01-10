use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use gtk4::glib;
use gtk4::prelude::*;

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

#[unsafe(no_mangle)]
pub unsafe extern "C" fn utils_get_diary_folder() -> *mut c_char {
    match crate::utils::get_diary_folder() {
        Ok(folder) => {
            match CString::new(folder) {
                Ok(c_str) => c_str.into_raw(),
                Err(_) => std::ptr::null_mut(),
            }
        }
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(1);
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn utils_get_photos_folder(
    basename: *const c_char,
    absolute: c_int,
) -> *mut c_char {
    if basename.is_null() {
        eprintln!("Error: utils_get_photos_folder called with null basename");
        return std::ptr::null_mut();
    }

    let basename_str = unsafe {
        match CStr::from_ptr(basename).to_str() {
            Ok(s) => s,
            Err(_) => {
                eprintln!("Invalid UTF-8 in basename");
                return std::ptr::null_mut();
            }
        }
    };

    // Get the window for error dialogs
    let window = gtk4::gio::Application::default()
        .and_downcast::<gtk4::Application>()
        .and_then(|app| app.active_window());

    match crate::utils::get_photos_folder(basename_str, absolute != 0, window.as_ref()) {
        Some(path) => {
            match CString::new(path) {
                Ok(c_str) => c_str.into_raw(),
                Err(_) => std::ptr::null_mut(),
            }
        }
        None => std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn utils_apply_color_scheme() {
    crate::utils::apply_color_scheme();
}
