use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use glib::translate::ToGlibPtr;
use std::cell::RefCell;

// FFI binding for GtkMdViewer
unsafe extern "C" {
    fn gtk_md_viewer_new_with_img_prefix(
        markdown: *const std::os::raw::c_char,
        img_prefix: *const std::os::raw::c_char,
    ) -> *mut gtk4::ffi::GtkWidget;
}

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_view.ui")]
    pub struct EntryView {
        #[template_child]
        pub edit_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub rename_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub delete_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub md_viewport: TemplateChild<gtk4::Viewport>,

        pub entry: RefCell<Option<crate::entry::Entry>>,
        pub md_view: RefCell<Option<gtk4::Widget>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryView {
        const NAME: &'static str = "EntryView";
        type Type = super::EntryView;
        type ParentType = gtk4::Box;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryView {
        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("deleted")
                        .param_types([crate::entry::Entry::static_type()])
                        .build(),
                ]
            })
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            // Connect button signals
            self.edit_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.edit_button_clicked()
            ));

            self.rename_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.rename_button_clicked()
            ));

            self.delete_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.delete_button_clicked()
            ));

            // Reload markdown when widget is mapped
            obj.connect_map(|obj| {
                obj.load_markdown();
            });
        }
    }

    impl WidgetImpl for EntryView {}
    impl BoxImpl for EntryView {}
}

glib::wrapper! {
    pub struct EntryView(ObjectSubclass<imp::EntryView>)
        @extends gtk4::Box, gtk4::Widget;
}

impl EntryView {
    pub fn new(entry: &crate::entry::Entry) -> Self {
        let obj: Self = glib::Object::new();
        obj.imp().entry.replace(Some(entry.clone()));
        obj
    }

    fn load_markdown(&self) {
        let imp = self.imp();

        let entry_ref = imp.entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };

        let Ok(text_md) = entry.read() else {
            return;
        };

        let folder = entry.property::<Option<String>>("folder")
            .unwrap_or_default();

        unsafe {
            use std::ffi::CString;

            let text_c = CString::new(text_md).unwrap();
            let folder_c = CString::new(folder).unwrap();

            let md_view_ptr = gtk_md_viewer_new_with_img_prefix(
                text_c.as_ptr(),
                folder_c.as_ptr(),
            );

            let md_view: gtk4::Widget = glib::translate::from_glib_none(md_view_ptr);
            imp.md_view.replace(Some(md_view.clone()));
            imp.md_viewport.set_child(Some(&md_view));
        }
    }

    fn edit_button_clicked(&self) {
        let entry_ref = self.imp().entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };

        let entry_edit = crate::widgets::entry_edit::EntryEdit::new(entry);

        let window = self.root().and_downcast::<gtk4::Window>();
        if let Some(_window) = window {
            // Assuming there's a way to push view to the window
            // This might need to be exposed via FFI if DiaryWindow methods aren't available
            // For now, we'll need to call the C function
            unsafe extern "C" {
                fn diary_window_get_instance() -> *mut gtk4::ffi::GtkWidget;
                fn diary_window_push_view(window: *mut gtk4::ffi::GtkWidget, view: *mut gtk4::ffi::GtkWidget);
            }

            unsafe {
                let diary_window = diary_window_get_instance();
                let view: gtk4::Widget = entry_edit.upcast();
                diary_window_push_view(diary_window, view.to_glib_none().0);
            }
        }
    }

    fn rename_button_clicked(&self) {
        let entry_ref = self.imp().entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };

        let basename = entry.property::<Option<String>>("basename")
            .unwrap_or_default();

        let window = self.root().and_downcast::<gtk4::Window>();
        let Some(window) = window else {
            return;
        };

        let dialog = crate::widgets::entry_rename_dialog::EntryRenameDialog::new(&basename);

        let entry_clone = entry.clone();
        dialog.connect_closure(
            "confirmed",
            false,
            glib::closure_local!(move |_dialog: crate::widgets::entry_rename_dialog::EntryRenameDialog, name: String| {
                let name_with_extension = format!("{}.md", name);
                println!("Renaming entry to '{}'", name_with_extension);
                let _ = entry_clone.rename_file(&name_with_extension);
            }),
        );

        dialog.open(&window);
    }

    fn delete_button_clicked(&self) {
        let entry_ref = self.imp().entry.borrow();
        let Some(entry) = entry_ref.as_ref() else {
            return;
        };

        let window = self.root().and_downcast::<gtk4::Window>();
        let Some(window) = window else {
            return;
        };

        let dialog = crate::widgets::entry_delete_dialog::EntryDeleteDialog::new();

        let entry_clone = entry.clone();
        let self_weak = self.downgrade();

        dialog.connect_closure(
            "confirmed",
            false,
            glib::closure_local!(move |_dialog: crate::widgets::entry_delete_dialog::EntryDeleteDialog| {
                let Some(this) = self_weak.upgrade() else { return };

                // Emit deleted signal before deleting the entry
                this.emit_by_name::<()>("deleted", &[&entry_clone]);

                // Delete the entry
                let _ = entry_clone.delete();
            }),
        );

        dialog.open(&window);
    }
}
