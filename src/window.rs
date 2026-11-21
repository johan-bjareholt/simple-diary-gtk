use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::RefCell;
use std::sync::atomic::{AtomicUsize, Ordering};

use crate::headerbuttons;
use crate::widgets::entry_browser::EntryBrowser;
use crate::widgets::settings_view::SettingsView;

// Raw pointer to the singleton; the app is single-threaded (GTK main thread)
static INSTANCE: AtomicUsize = AtomicUsize::new(0);

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/window.ui")]
    pub struct DiaryWindow {
        #[template_child]
        pub content_stack: TemplateChild<gtk4::Stack>,
        #[template_child]
        pub new_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub back_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub settings_button: TemplateChild<gtk4::Button>,

        pub view_stack: RefCell<Vec<gtk4::Widget>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for DiaryWindow {
        const NAME: &'static str = "DiaryWindow";
        type Type = super::DiaryWindow;
        type ParentType = adw::ApplicationWindow;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for DiaryWindow {
        fn dispose(&self) {
            // Remove all views without touching the header buttons,
            // which may already be gone during shutdown
            while let Some(view) = self.view_stack.borrow_mut().pop() {
                self.content_stack.remove(&view);
            }
            // dispose chains up to the parent automatically
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            obj.push_view(&EntryBrowser::new().upcast::<gtk4::Widget>());

            self.new_button.connect_clicked(glib::clone!(
                #[weak]
                obj,
                move |_| obj.new_button_pressed()
            ));
            self.back_button.connect_clicked(glib::clone!(
                #[weak]
                obj,
                move |_| obj.back_button_pressed()
            ));
            self.settings_button.connect_clicked(glib::clone!(
                #[weak]
                obj,
                move |_| obj.settings_button_pressed()
            ));
        }
    }

    impl WidgetImpl for DiaryWindow {}
    impl gtk4::subclass::application_window::ApplicationWindowImpl for DiaryWindow {}
    impl gtk4::subclass::window::WindowImpl for DiaryWindow {}
    impl adw::subclass::prelude::AdwApplicationWindowImpl for DiaryWindow {}
}

glib::wrapper! {
    pub struct DiaryWindow(ObjectSubclass<imp::DiaryWindow>)
        @extends adw::ApplicationWindow, gtk4::ApplicationWindow, gtk4::Window, gtk4::Widget,
        @implements gtk4::Native, gtk4::ShortcutManager, gtk4::Root,
        gtk4::ConstraintTarget, gtk4::Buildable, gtk4::Accessible,
        gtk4::gio::ActionGroup, gtk4::gio::ActionMap;
}

impl DiaryWindow {
    pub fn new(application: &gtk4::Application) -> Self {
        let window: Self = glib::Object::builder()
            .property("application", application)
            .build();
        assert_eq!(INSTANCE.swap(window.as_ptr() as usize, Ordering::SeqCst), 0, "DiaryWindow already created");
        window
    }

    pub fn instance() -> Self {
        let ptr = INSTANCE.load(Ordering::SeqCst);
        assert!(ptr != 0, "DiaryWindow not created");
        let widget: gtk4::Widget = unsafe { glib::translate::from_glib_none(ptr as *mut gtk4::ffi::GtkWidget) };
        widget.downcast::<DiaryWindow>().expect("Not a DiaryWindow")
    }

    pub fn push_view(&self, new_view: &gtk4::Widget) {
        let imp = self.imp();
        let prev_view = imp.view_stack.borrow().last().cloned();

        imp.view_stack.borrow_mut().push(new_view.clone());
        imp.content_stack.add_child(new_view);
        imp.content_stack.set_visible_child(new_view);

        self.update_header_buttons(new_view, prev_view.as_ref());
    }

    pub fn pop_view(&self) {
        let imp = self.imp();
        let Some(current_view) = imp.view_stack.borrow_mut().pop() else {
            return;
        };
        let Some(prev_view) = imp.view_stack.borrow().last().cloned() else {
            return;
        };

        self.update_header_buttons(&prev_view, Some(&current_view));

        imp.content_stack.remove(&current_view);
        imp.content_stack.set_visible_child(&prev_view);
    }

    fn update_header_buttons(&self, new_view: &gtk4::Widget, old_view: Option<&gtk4::Widget>) {
        let imp = self.imp();
        let new_button = imp.new_button.as_ptr();
        let back_button = imp.back_button.as_ptr();
        let settings_button = imp.settings_button.as_ptr();

        if new_button.is_null() || back_button.is_null() || settings_button.is_null() {
            return;
        }

        /* on leave */
        if let Some(old_view) = old_view {
            match headerbuttons::peek_interface(old_view) {
                Some(iface) => (iface.on_leave)(old_view.as_ptr(), new_button, back_button, settings_button),
                None => crate::headerbuttons_bindings::header_buttons_control_default_on_leave(
                    old_view.as_ptr(),
                    new_button,
                    back_button,
                    settings_button,
                ),
            }
        }

        /* on enter */
        match headerbuttons::peek_interface(new_view) {
            Some(iface) => (iface.on_enter)(new_view.as_ptr(), new_button, back_button, settings_button),
            None => crate::headerbuttons_bindings::header_buttons_control_default_on_enter(
                new_view.as_ptr(),
                new_button,
                back_button,
                settings_button,
            ),
        }
    }

    fn current_view(&self) -> Option<gtk4::Widget> {
        self.imp().view_stack.borrow().last().cloned()
    }

    fn new_button_pressed(&self) {
        let Some(view) = self.current_view() else {
            return;
        };
        match headerbuttons::peek_interface(&view) {
            Some(iface) => (iface.on_new_pressed)(view.as_ptr()),
            None => crate::headerbuttons_bindings::header_buttons_control_default_on_new_pressed(
                view.as_ptr(),
            ),
        }
    }

    fn back_button_pressed(&self) {
        let Some(view) = self.current_view() else {
            return;
        };
        match headerbuttons::peek_interface(&view) {
            Some(iface) => (iface.on_back_pressed)(view.as_ptr()),
            // The C default simply popped the view
            None => self.pop_view(),
        }
    }

    fn settings_button_pressed(&self) {
        self.push_view(&SettingsView::new().upcast::<gtk4::Widget>());
    }
}
