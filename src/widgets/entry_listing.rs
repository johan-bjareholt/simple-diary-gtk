use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::RefCell;

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_listing.ui")]
    pub struct EntryListing {
        #[template_child]
        pub name_label: TemplateChild<gtk4::Label>,

        pub entry: RefCell<Option<crate::entry::Entry>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryListing {
        const NAME: &'static str = "EntryListing";
        type Type = super::EntryListing;
        type ParentType = gtk4::Box;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryListing {
        fn properties() -> &'static [glib::ParamSpec] {
            use std::sync::OnceLock;
            static PROPERTIES: OnceLock<Vec<glib::ParamSpec>> = OnceLock::new();
            PROPERTIES.get_or_init(|| {
                vec![
                    glib::ParamSpecObject::builder::<crate::entry::Entry>("entry")
                        .construct_only()
                        .build(),
                ]
            })
        }

        fn set_property(&self, _id: usize, value: &glib::Value, pspec: &glib::ParamSpec) {
            match pspec.name() {
                "entry" => {
                    let entry = value.get::<Option<crate::entry::Entry>>().unwrap();
                    self.entry.replace(entry);
                }
                _ => unimplemented!(),
            }
        }

        fn property(&self, _id: usize, pspec: &glib::ParamSpec) -> glib::Value {
            match pspec.name() {
                "entry" => self.entry.borrow().to_value(),
                _ => unimplemented!(),
            }
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            if let Some(entry) = self.entry.borrow().as_ref() {
                obj.update_title();

                // Update the title when the entry is renamed
                entry.connect_notify_local(
                    Some("filename"),
                    glib::clone!(
                        #[weak]
                        obj,
                        move |_entry, _pspec| obj.update_title()
                    ),
                );
            }
        }
    }

    impl WidgetImpl for EntryListing {}
    impl BoxImpl for EntryListing {}
}

glib::wrapper! {
    pub struct EntryListing(ObjectSubclass<imp::EntryListing>)
        @extends gtk4::Box, gtk4::Widget,
        @implements gtk4::ConstraintTarget, gtk4::Buildable, gtk4::Accessible;
}

impl EntryListing {
    pub fn new(entry: &crate::entry::Entry) -> Self {
        glib::Object::builder()
            .property("entry", entry)
            .build()
    }

    pub fn entry(&self) -> crate::entry::Entry {
        self.imp().entry.borrow().clone().expect("entry not set")
    }

    fn update_title(&self) {
        let imp = self.imp();
        let entry_ref = imp.entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };

        let basename = entry.property::<Option<String>>("basename").unwrap_or_default();
        imp.name_label.set_label(&basename);
    }
}
