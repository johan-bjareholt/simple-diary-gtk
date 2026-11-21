use gtk4::glib;
use gtk4::prelude::*;
use glib::translate::{IntoGlib, ToGlibPtr};
use glib::ffi::GType;
use glib::gobject_ffi;

use crate::window::DiaryWindow;

// Safe native API for the still-C main.c and other callers

/// Get the singleton DiaryWindow instance
pub fn instance() -> DiaryWindow {
    DiaryWindow::instance()
}

/// Push a new view onto the window's view stack
pub fn push_view(view: &gtk4::Widget) {
    DiaryWindow::instance().push_view(view);
}

/// Pop the current view off the window's view stack
pub fn pop_view() {
    DiaryWindow::instance().pop_view();
}

// FFI exports for C callers (main.c, headerbuttons.c)

#[unsafe(no_mangle)]
pub extern "C" fn diary_window_get_type() -> GType {
    DiaryWindow::static_type().into_glib()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn diary_window_new(application: *mut gtk4::ffi::GtkApplication) -> *mut gtk4::ffi::GtkApplicationWindow {
    if application.is_null() {
        return std::ptr::null_mut();
    }

    if !gtk4::is_initialized() {
        gtk4::init().expect("Failed to initialize GTK from Rust");
    }

    let app: gtk4::Application = unsafe { glib::translate::from_glib_none(application) };
    let window = DiaryWindow::new(&app);
    let widget: gtk4::Widget = window.upcast();
    let window: gtk4::ApplicationWindow = widget.downcast::<gtk4::ApplicationWindow>().expect("Not an ApplicationWindow");
    window.to_glib_full()
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn diary_window_get_instance() -> *mut gobject_ffi::GObject {
    DiaryWindow::instance().upcast_ref::<glib::Object>().to_glib_none().0
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn diary_window_push_view(window: *mut gtk4::ffi::GtkWidget, view: *mut gtk4::ffi::GtkWidget) {
    if window.is_null() || view.is_null() {
        return;
    }
    let window: gtk4::Widget = unsafe { glib::translate::from_glib_none(window) };
    let window = window.downcast::<DiaryWindow>().expect("Not a DiaryWindow");
    let view: gtk4::Widget = unsafe { glib::translate::from_glib_none(view) };
    window.push_view(&view);
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn diary_window_pop_view(window: *mut gtk4::ffi::GtkWidget) {
    if window.is_null() {
        return;
    }
    let window: gtk4::Widget = unsafe { glib::translate::from_glib_none(window) };
    let window = window.downcast::<DiaryWindow>().expect("Not a DiaryWindow");
    window.pop_view();
}
