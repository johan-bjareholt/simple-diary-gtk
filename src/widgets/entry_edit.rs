use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::{Cell, RefCell};

// FFI binding for GtkMdEditor
unsafe extern "C" {
    fn gtk_md_editor_new() -> *mut gtk4::ffi::GtkWidget;
}

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_edit.ui")]
    pub struct EntryEdit {
        #[template_child]
        pub entry_edit_viewport: TemplateChild<gtk4::ScrolledWindow>,
        #[template_child]
        pub add_image_button: TemplateChild<gtk4::Button>,

        pub entry: RefCell<Option<crate::entry::Entry>>,
        pub md_editor: RefCell<Option<gtk4::Widget>>,
        pub unsaved_changes: Cell<bool>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryEdit {
        const NAME: &'static str = "EntryEdit";
        type Type = super::EntryEdit;
        type ParentType = gtk4::Box;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryEdit {
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
            
            // Create markdown editor
            let md_editor_ptr = unsafe { gtk_md_editor_new() };
            let md_editor: gtk4::Widget = unsafe { 
                glib::translate::from_glib_none(md_editor_ptr) 
            };
            self.md_editor.replace(Some(md_editor.clone()));
            self.entry_edit_viewport.set_child(Some(&md_editor));
            
            // Read entry content
            obj.read_entry();
            
            // Connect signals
            self.add_image_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.add_image_button_clicked()
            ));
            
            obj.connect_destroy(|obj| {
                obj.save_entry();
            });
        }
    }

    impl WidgetImpl for EntryEdit {}
    impl BoxImpl for EntryEdit {}
}

glib::wrapper! {
    pub struct EntryEdit(ObjectSubclass<imp::EntryEdit>)
        @extends gtk4::Box, gtk4::Widget;
}

impl EntryEdit {
    pub fn new(entry: &crate::entry::Entry) -> Self {
        glib::Object::builder()
            .property("entry", entry)
            .build()
    }

    fn read_entry(&self) {
        let imp = self.imp();
        
        let entry_ref = imp.entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };
        
        let editor_ref = imp.md_editor.borrow();
        let Some(md_editor) = editor_ref.as_ref() else {
            return;
        };
        
        match entry.read() {
            Ok(body) => {
                let text_view = md_editor.downcast_ref::<gtk4::TextView>()
                    .expect("md_editor is not a TextView");
                let buffer = text_view.buffer();
                buffer.set_text(&body);
            }
            Err(e) => {
                eprintln!("Failed to read entry: {}", e);
            }
        }
    }

    pub fn save_entry(&self) {
        let imp = self.imp();
        
        if !imp.unsaved_changes.get() {
            return;
        }
        
        let entry_ref = imp.entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };
        
        let editor_ref = imp.md_editor.borrow();
        let Some(md_editor) = editor_ref.as_ref() else {
            return;
        };
        
        let text_view = md_editor.downcast_ref::<gtk4::TextView>()
            .expect("md_editor is not a TextView");
        let buffer = text_view.buffer();
        let start = buffer.start_iter();
        let end = buffer.end_iter();
        let text = buffer.text(&start, &end, false);
        
        if let Err(e) = entry.write(&text) {
            eprintln!("Failed to save entry: {}", e);
            return;
        }
        
        imp.unsaved_changes.set(false);
    }

    fn add_image_button_clicked(&self) {
        let imp = self.imp();
        
        let entry_ref = imp.entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };
        
        let basename = entry.property::<Option<String>>("basename")
            .unwrap_or_default();
        
        let window = self.root().and_downcast::<gtk4::Window>();
        let Some(window) = window else {
            return;
        };
        
        let image_picker = crate::widgets::image_picker::ImagePicker::new(&basename);
        
        image_picker.connect_closure(
            "image-selected",
            false,
            glib::closure_local!(
                #[weak(rename_to = this)] self,
                move |_picker: crate::widgets::image_picker::ImagePicker, name: String, path: String| {
                    this.add_image(&name, &path);
                }
            ),
        );
        
        image_picker.open(&window);
    }

    fn add_image(&self, name: &str, path: &str) {
        println!("Adding image named '{}' from path '{}'", name, path);
        
        let imp = self.imp();
        let editor_ref = imp.md_editor.borrow();
        let Some(md_editor) = editor_ref.as_ref() else {
            return;
        };
        
        let text_view = md_editor.downcast_ref::<gtk4::TextView>()
            .expect("md_editor is not a TextView");
        let buffer = text_view.buffer();
        let md_image_link = format!("![{}](<{}>)", name, path);
        buffer.insert_at_cursor(&md_image_link);
    }

    pub fn on_back_pressed(&self) {
        self.save_entry();
    }
}
