use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_rename_dialog.ui")]
    pub struct EntryRenameDialog {
        #[template_child]
        pub name_entry: TemplateChild<gtk4::Entry>,
        #[template_child]
        pub button_cancel: TemplateChild<gtk4::Button>,
        #[template_child]
        pub button_ok: TemplateChild<gtk4::Button>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryRenameDialog {
        const NAME: &'static str = "EntryRenameDialog";
        type Type = super::EntryRenameDialog;
        type ParentType = gtk4::Window;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();

            klass.install_action("dialog.confirm", None, move |obj, _, _| {
                obj.handle_confirm();
            });

            klass.install_action("dialog.cancel", None, move |obj, _, _| {
                obj.handle_cancel();
            });
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryRenameDialog {
        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("confirmed")
                        .param_types([String::static_type()])
                        .build(),
                    glib::subclass::Signal::builder("cancelled")
                        .build(),
                ]
            })
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            self.button_ok.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "dialog.confirm", None).unwrap()
            ));

            self.button_cancel.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "dialog.cancel", None).unwrap()
            ));

            self.name_entry.connect_activate(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "dialog.confirm", None).unwrap()
            ));
        }
    }

    impl WidgetImpl for EntryRenameDialog {}
    impl WindowImpl for EntryRenameDialog {}
}

glib::wrapper! {
    pub struct EntryRenameDialog(ObjectSubclass<imp::EntryRenameDialog>)
        @extends gtk4::Window, gtk4::Widget,
        @implements gtk4::gio::ActionGroup, gtk4::gio::ActionMap, gtk4::Accessible, gtk4::Actionable,
                    gtk4::Buildable, gtk4::ConstraintTarget, gtk4::ShortcutManager, gtk4::Root, gtk4::Native;
}

impl EntryRenameDialog {
    pub fn new(basename: &str) -> Self {
        let obj: Self = glib::Object::new();

        let buffer = gtk4::EntryBuffer::new(Some(basename));
        obj.imp().name_entry.set_buffer(&buffer);

        obj
    }

    pub fn get_name(&self) -> String {
        let buffer = self.imp().name_entry.buffer();
        buffer.text().to_string()
    }

    fn validate_name(&self, name: &str) -> bool {
        if name.contains('.') || name.contains('/') {
            crate::utils::error_dialog(self, "Slashes and dots are not allowed in titles");
            false
        } else {
            true
        }
    }

    fn handle_confirm(&self) {
        let name = self.get_name();

        if !self.validate_name(&name) {
            return;
        }

        self.emit_by_name::<()>("confirmed", &[&name]);
        self.close();
    }

    fn handle_cancel(&self) {
        self.emit_by_name::<()>("cancelled", &[]);
        self.close();
    }

    pub fn open(&self, parent: &gtk4::Window) {
        self.set_transient_for(Some(parent));
        self.set_visible(true);
    }
}
