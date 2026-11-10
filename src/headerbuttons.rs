use glib::subclass::prelude::*;
use glib::translate::*;
use gtk4::prelude::*;

pub(crate) mod ffi {
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

/// Trait implemented by widgets that want to handle header button events.
/// Use together with [`impl_header_buttons_control!`].
pub trait HeaderButtonsHandler {
    /// Called when entering the view
    fn on_enter(&self, new_button: &gtk4::Button, back_button: &gtk4::Button, settings_button: &gtk4::Button);
    /// Called when leaving the view (either back or forward to some new page)
    fn on_leave(&self, new_button: &gtk4::Button, back_button: &gtk4::Button, settings_button: &gtk4::Button);
    /// Called when pressing new
    fn on_new_pressed(&self);
    /// Called when pressing back
    fn on_back_pressed(&self);
}

/// Implements the `HeaderButtonsControl` interface for a widget, wiring the
/// interface vfuncs to the widget's [`HeaderButtonsHandler`] implementation.
///
/// Usage (from the widget's module, where `imp` is the widget's `imp` module):
/// ```ignore
/// impl_header_buttons_control!(MyWidget, imp::MyWidget);
/// ```
/// and declare `type Interfaces = (HeaderButtonsControl,);` in `ObjectSubclass`.
#[macro_export]
macro_rules! impl_header_buttons_control {
    ($widget:ty, $imp:ty) => {
        mod header_buttons_iface_glue {
            use super::*;
            use gtk4::glib::translate::{from_glib_borrow, Borrowed};

            pub(crate) extern "C" fn on_enter(
                widget: *mut gtk4::ffi::GtkWidget,
                new_button: *mut gtk4::ffi::GtkButton,
                back_button: *mut gtk4::ffi::GtkButton,
                settings_button: *mut gtk4::ffi::GtkButton,
            ) {
                let widget_rs: Borrowed<gtk4::Widget> = unsafe { from_glib_borrow(widget) };
                let new_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(new_button) };
                let back_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(back_button) };
                let settings_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(settings_button) };

                if let Some(obj) = widget_rs.dynamic_cast_ref::<$widget>() {
                    <$widget as crate::headerbuttons::HeaderButtonsHandler>::on_enter(
                        obj,
                        &new_button_rs,
                        &back_button_rs,
                        &settings_button_rs,
                    );
                }
            }

            pub(crate) extern "C" fn on_leave(
                widget: *mut gtk4::ffi::GtkWidget,
                new_button: *mut gtk4::ffi::GtkButton,
                back_button: *mut gtk4::ffi::GtkButton,
                settings_button: *mut gtk4::ffi::GtkButton,
            ) {
                let widget_rs: Borrowed<gtk4::Widget> = unsafe { from_glib_borrow(widget) };
                let new_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(new_button) };
                let back_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(back_button) };
                let settings_button_rs: Borrowed<gtk4::Button> = unsafe { from_glib_borrow(settings_button) };

                if let Some(obj) = widget_rs.dynamic_cast_ref::<$widget>() {
                    <$widget as crate::headerbuttons::HeaderButtonsHandler>::on_leave(
                        obj,
                        &new_button_rs,
                        &back_button_rs,
                        &settings_button_rs,
                    );
                }
            }

            pub(crate) extern "C" fn on_new_pressed(widget: *mut gtk4::ffi::GtkWidget) {
                let widget_rs: Borrowed<gtk4::Widget> = unsafe { from_glib_borrow(widget) };

                if let Some(obj) = widget_rs.dynamic_cast_ref::<$widget>() {
                    <$widget as crate::headerbuttons::HeaderButtonsHandler>::on_new_pressed(obj);
                }
            }

            pub(crate) extern "C" fn on_back_pressed(widget: *mut gtk4::ffi::GtkWidget) {
                let widget_rs: Borrowed<gtk4::Widget> = unsafe { from_glib_borrow(widget) };

                if let Some(obj) = widget_rs.dynamic_cast_ref::<$widget>() {
                    <$widget as crate::headerbuttons::HeaderButtonsHandler>::on_back_pressed(obj);
                }
            }
        }

        unsafe impl gtk4::glib::subclass::prelude::IsImplementable<$imp>
            for crate::headerbuttons::HeaderButtonsControl
        {
            fn interface_init(iface: &mut gtk4::glib::Interface<Self>) {
                let iface = iface.as_mut();
                iface.on_enter = header_buttons_iface_glue::on_enter;
                iface.on_leave = header_buttons_iface_glue::on_leave;
                iface.on_new_pressed = header_buttons_iface_glue::on_new_pressed;
                iface.on_back_pressed = header_buttons_iface_glue::on_back_pressed;
            }
        }
    };
}
