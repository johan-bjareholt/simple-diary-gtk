use std::ffi::CStr;
use std::os::raw::c_char;
use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{ToGlibPtr, FromGlibPtrNone};

use super::image_picker::ImagePicker;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn image_picker_dialog_new(basename: *const c_char) -> *mut gtk4::ffi::GtkWindow {
    if basename.is_null() {
        return std::ptr::null_mut();
    }
    
    // Ensure GTK is initialized from Rust side
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }
    
    let basename_str = unsafe {
        CStr::from_ptr(basename).to_str().unwrap_or("")
    };
    
    let picker = ImagePicker::new(basename_str);
    let window: gtk4::Window = picker.upcast();
    window.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn image_picker_dialog_open(
    picker: *mut gtk4::ffi::GtkWindow,
    parent: *mut gtk4::ffi::GtkWindow,
) {
    if picker.is_null() || parent.is_null() {
        return;
    }
    
    let window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(picker)
    };
    let picker = window.downcast::<ImagePicker>().expect("Not an ImagePicker");
    
    let parent_window: gtk4::Window = unsafe {
        FromGlibPtrNone::from_glib_none(parent)
    };
    
    picker.open(&parent_window);
}
