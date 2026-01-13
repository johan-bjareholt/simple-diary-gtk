use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_delete_dialog.ui")]
    pub struct EntryDeleteDialog {
        #[template_child]
        pub button_cancel: TemplateChild<gtk4::Button>,
        #[template_child]
        pub button_ok: TemplateChild<gtk4::Button>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryDeleteDialog {
        const NAME: &'static str = "EntryDeleteDialog";
        type Type = super::EntryDeleteDialog;
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

    impl ObjectImpl for EntryDeleteDialog {
        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("confirmed")
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
        }
    }

    impl WidgetImpl for EntryDeleteDialog {}
    impl WindowImpl for EntryDeleteDialog {}
}

glib::wrapper! {
    pub struct EntryDeleteDialog(ObjectSubclass<imp::EntryDeleteDialog>)
        @extends gtk4::Window, gtk4::Widget,
        @implements gtk4::gio::ActionGroup, gtk4::gio::ActionMap, gtk4::Accessible, gtk4::Actionable,
                    gtk4::Buildable, gtk4::ConstraintTarget, gtk4::ShortcutManager, gtk4::Root, gtk4::Native;
}

impl EntryDeleteDialog {
    pub fn new() -> Self {
        glib::Object::new()
    }

    fn handle_confirm(&self) {
        self.emit_by_name::<()>("confirmed", &[]);
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
