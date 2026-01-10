use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{ToGlibPtr, FromGlibPtrNone, IntoGlib};
use glib::gobject_ffi;
use glib::ffi::GType;

use super::entry::Entry;

#[unsafe(no_mangle)]
pub extern "C" fn entry_get_type() -> GType {
    Entry::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_new(
    folder: *const c_char,
    filename: *const c_char,
) -> *mut gobject_ffi::GObject {
    if folder.is_null() || filename.is_null() {
        return std::ptr::null_mut();
    }

    // Ensure GTK is initialized
    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let folder_str = unsafe { CStr::from_ptr(folder).to_str().unwrap_or("") };
    let filename_str = unsafe { CStr::from_ptr(filename).to_str().unwrap_or("") };

    let entry = Entry::new(folder_str, filename_str);
    entry.upcast::<glib::Object>().to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_read(
    entry: *mut gobject_ffi::GObject,
    _err: *mut *mut glib::ffi::GError,
) -> *mut c_char {
    if entry.is_null() {
        return std::ptr::null_mut();
    }

    let obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = obj.downcast::<Entry>().expect("Not an Entry");

    match entry.read() {
        Ok(content) => {
            match CString::new(content) {
                Ok(c_str) => c_str.into_raw(),
                Err(_) => std::ptr::null_mut(),
            }
        }
        Err(e) => {
            eprintln!("{}", e);
            std::ptr::null_mut()
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_write(
    entry: *mut gobject_ffi::GObject,
    text: *const c_char,
    _err: *mut *mut glib::ffi::GError,
) -> i32 {
    if entry.is_null() || text.is_null() {
        return 0;
    }

    let obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = obj.downcast::<Entry>().expect("Not an Entry");

    let text_str = unsafe {
        CStr::from_ptr(text).to_str().unwrap_or("")
    };

    match entry.write(text_str) {
        Ok(_) => 1,
        Err(e) => {
            eprintln!("{}", e);
            0
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_rename_file(
    entry: *mut gobject_ffi::GObject,
    new_name: *const c_char,
) -> i32 {
    if entry.is_null() || new_name.is_null() {
        return 0;
    }

    let obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = obj.downcast::<Entry>().expect("Not an Entry");

    let new_name_str = unsafe {
        CStr::from_ptr(new_name).to_str().unwrap_or("")
    };

    match entry.rename_file(new_name_str) {
        Ok(_) => 1,
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(1);
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn entry_delete(entry: *mut gobject_ffi::GObject) -> i32 {
    if entry.is_null() {
        return 0;
    }

    let obj: glib::Object = unsafe { FromGlibPtrNone::from_glib_none(entry) };
    let entry = obj.downcast::<Entry>().expect("Not an Entry");

    match entry.delete() {
        Ok(_) => 1,
        Err(e) => {
            eprintln!("{}", e);
            0
        }
    }
}
