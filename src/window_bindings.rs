use glib::translate::{from_glib_none, ToGlibPtr};

// DiaryWindow is still implemented in C
#[allow(unsafe_op_in_unsafe_fn)]
unsafe extern "C" {
    fn diary_window_get_instance() -> *mut gtk4::ffi::GtkWidget;
    fn diary_window_push_view(window: *mut gtk4::ffi::GtkWidget, view: *mut gtk4::ffi::GtkWidget);
    fn diary_window_pop_view(window: *mut gtk4::ffi::GtkWidget);
}

/// Get the singleton DiaryWindow instance
pub fn instance() -> gtk4::Widget {
    unsafe { from_glib_none(diary_window_get_instance()) }
}

/// Push a new view onto the window's view stack
pub fn push_view(view: &gtk4::Widget) {
    unsafe {
        diary_window_push_view(diary_window_get_instance(), view.to_glib_none().0);
    }
}

/// Pop the current view off the window's view stack
pub fn pop_view() {
    unsafe {
        diary_window_pop_view(diary_window_get_instance());
    }
}
