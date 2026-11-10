use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;

use super::entry_listing::EntryListing;

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_list.ui")]
    pub struct EntryList {
        #[template_child]
        pub entry_list_box: TemplateChild<gtk4::ListBox>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryList {
        const NAME: &'static str = "EntryList";
        type Type = super::EntryList;
        type ParentType = gtk4::Box;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryList {
        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("selection-changed")
                        .param_types([crate::entry::Entry::static_type()])
                        .build(),
                ]
            })
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();
            let list_box = &self.entry_list_box;

            list_box.set_selection_mode(gtk4::SelectionMode::None);

            // Necessary for making no entry selected by default
            list_box.connect_row_activated(move |list_box, row| {
                list_box.set_selection_mode(gtk4::SelectionMode::Single);
                list_box.select_row(Some(row));
            });

            list_box.connect_selected_rows_changed(glib::clone!(
                #[weak]
                obj,
                move |list_box| {
                    let entry: Option<crate::entry::Entry> =
                        list_box.selected_row().map(|row| row_entry(&row));

                    match &entry {
                        Some(entry) => {
                            let name = entry.property::<Option<String>>("basename");
                            println!("{}", name.unwrap_or_default());
                        }
                        None => println!("entry unselected"),
                    }

                    obj.emit_by_name::<()>("selection-changed", &[&entry.to_value()]);
                }
            ));

            obj.load_entry_list();

            list_box.set_sort_func(|row1, row2| {
                let name1 = row_entry(row1)
                    .property::<Option<String>>("filename")
                    .unwrap_or_default();
                let name2 = row_entry(row2)
                    .property::<Option<String>>("filename")
                    .unwrap_or_default();
                // Descending by filename
                match name2.cmp(&name1) {
                    std::cmp::Ordering::Less => gtk4::Ordering::Larger,
                    std::cmp::Ordering::Equal => gtk4::Ordering::Equal,
                    std::cmp::Ordering::Greater => gtk4::Ordering::Smaller,
                }
            });

            obj.set_vexpand(true);
            obj.set_hexpand(true);
        }
    }

    impl WidgetImpl for EntryList {}
    impl BoxImpl for EntryList {}
}

glib::wrapper! {
    pub struct EntryList(ObjectSubclass<imp::EntryList>)
        @extends gtk4::Box, gtk4::Widget,
        @implements gtk4::ConstraintTarget, gtk4::Buildable, gtk4::Accessible;
}

fn row_entry(row: &gtk4::ListBoxRow) -> crate::entry::Entry {
    row.child()
        .expect("entry list row has no child")
        .downcast_ref::<EntryListing>()
        .expect("Not an EntryListing")
        .entry()
}

/// List the .md files in the diary folder
fn list_entries(dir_path: &str) -> Vec<String> {
    let mut files = Vec::new();
    if let Ok(read_dir) = std::fs::read_dir(dir_path) {
        for entry in read_dir.flatten() {
            let path = entry.path();
            if path.is_file() && path.extension().is_some_and(|ext| ext == "md") {
                if let Some(filename) = entry.file_name().to_str() {
                    files.push(filename.to_string());
                }
            }
        }
    }
    files
}

impl EntryList {
    pub fn new() -> Self {
        glib::Object::new()
    }

    fn load_entry_list(&self) {
        let folder = match crate::utils::get_diary_folder() {
            Ok(folder) => folder,
            Err(e) => {
                eprintln!("Failed to get diary folder: {}", e);
                return;
            }
        };

        for filename in list_entries(&folder) {
            let entry = crate::entry::Entry::new(&folder, &filename);
            self.add_entry(&entry);
        }
    }

    pub fn add_entry(&self, entry: &crate::entry::Entry) {
        let listing = EntryListing::new(entry);
        let row = gtk4::ListBoxRow::new();
        row.set_child(Some(&listing));
        self.imp().entry_list_box.insert(&row, -1);
    }

    /// Find the row for the entry with the given filename, if any
    pub fn find(&self, filename: &str) -> Option<gtk4::ListBoxRow> {
        let list_box = &self.imp().entry_list_box;
        let mut i = 0;
        while let Some(row) = list_box.row_at_index(i) {
            let name = row_entry(&row).property::<Option<String>>("filename");
            if name.as_deref() == Some(filename) {
                return Some(row);
            }
            i += 1;
        }
        None
    }

    /// Remove the row for the given entry. Returns false if it wasn't in the list.
    pub fn remove(&self, entry: &crate::entry::Entry) -> bool {
        let filename = entry.property::<Option<String>>("filename").unwrap_or_default();
        match self.find(&filename) {
            Some(row) => {
                self.imp().entry_list_box.remove(&row);
                true
            }
            None => false,
        }
    }

    pub fn focus(&self, row: &gtk4::ListBoxRow) {
        self.imp().entry_list_box.select_row(Some(row));
        let entry = row_entry(row);
        self.emit_by_name::<()>("selection-changed", &[&entry.to_value()]);
    }

    pub fn unfocus(&self) {
        self.imp().entry_list_box.select_row(None::<&gtk4::ListBoxRow>);
    }
}
