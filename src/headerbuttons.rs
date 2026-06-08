use glib::subclass::prelude::*;
use glib::translate::*;
use gtk4::prelude::*;

mod ffi {
    use super::*;

    #[repr(C)]
    pub struct Instance(std::ffi::c_void);

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct Interface {
        pub parent: glib::gobject_ffi::GTypeInterface,
        pub on_enter: extern "C" fn(_widget: *mut gtk4::ffi::GtkWidget, _new_button: *mut gtk4::ffi::GtkButton, _back_button: *mut gtk4::ffi::GtkButton, _settings_button: *mut gtk4::ffi::GtkButton),
        pub on_leave: extern "C" fn(_widget: *mut gtk4::ffi::GtkWidget, _new_button: *mut gtk4::ffi::GtkButton, _back_button: *mut gtk4::ffi::GtkButton, _settings_button: *mut gtk4::ffi::GtkButton),
        pub on_new_pressed: extern "C" fn(_widget: *mut gtk4::ffi::GtkWidget),
        pub on_back_pressed: extern "C" fn(_widget: *mut gtk4::ffi::GtkWidget),
    }

    unsafe impl InterfaceStruct for Interface {
        type Type = super::iface::HeaderButtonsControl;
    }
}

extern "C" fn default_on_enter(
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

extern "C" fn default_on_leave(
    _widget: *mut gtk4::ffi::GtkWidget,
    _new_button: *mut gtk4::ffi::GtkButton,
    _back_button: *mut gtk4::ffi::GtkButton,
    _settings_button: *mut gtk4::ffi::GtkButton,
) {
}

extern "C" fn default_on_new_pressed(_widget: *mut gtk4::ffi::GtkWidget) {}

extern "C" fn default_on_back_pressed(_widget: *mut gtk4::ffi::GtkWidget) {
    // The C side (headerbuttons.c) provides the default back-pressed handler
    // for widgets that don't implement the interface.
}

mod iface {
    use super::*;

    pub enum HeaderButtonsControl {}

    #[glib::object_interface]
    impl ObjectInterface for HeaderButtonsControl {
        const NAME: &'static str = "HeaderButtonsControl";
        type Instance = ffi::Instance;
        type Interface = ffi::Interface;

        // Called once per interface type
        fn interface_init(klass: &mut Self::Interface) {
            klass.on_enter = default_on_enter;
            klass.on_leave = default_on_leave;
            klass.on_new_pressed = default_on_new_pressed;
            klass.on_back_pressed = default_on_back_pressed;
        }
    }
}

glib::wrapper! {
    pub struct HeaderButtonsControl(ObjectInterface<iface::HeaderButtonsControl>);
}
