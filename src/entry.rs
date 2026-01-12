use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::RefCell;
use std::path::Path;

mod imp {
    use super::*;

    #[derive(Default)]
    pub struct Entry {
        pub folder: RefCell<Option<String>>,
        pub filename: RefCell<Option<String>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for Entry {
        const NAME: &'static str = "DiaryEntry";
        type Type = super::Entry;
        type ParentType = glib::Object;
    }

    impl ObjectImpl for Entry {
        fn properties() -> &'static [glib::ParamSpec] {
            use std::sync::OnceLock;
            static PROPERTIES: OnceLock<Vec<glib::ParamSpec>> = OnceLock::new();
            PROPERTIES.get_or_init(|| {
                vec![
                    glib::ParamSpecString::builder("folder")
                        .construct_only()
                        .build(),
                    glib::ParamSpecString::builder("filename")
                        .build(),
                    glib::ParamSpecString::builder("basename")
                        .read_only()
                        .build(),
                    glib::ParamSpecString::builder("filepath")
                        .read_only()
                        .build(),
                ]
            })
        }

        fn set_property(&self, _id: usize, value: &glib::Value, pspec: &glib::ParamSpec) {
            match pspec.name() {
                "folder" => {
                    let folder = value.get::<Option<String>>().unwrap();
                    self.folder.replace(folder);
                }
                "filename" => {
                    let filename = value.get::<Option<String>>().unwrap();
                    self.filename.replace(filename);
                }
                _ => unimplemented!(),
            }
        }

        fn property(&self, _id: usize, pspec: &glib::ParamSpec) -> glib::Value {
            match pspec.name() {
                "folder" => self.folder.borrow().to_value(),
                "filename" => self.filename.borrow().to_value(),
                "basename" => {
                    let filename = self.filename.borrow();
                    filename.as_ref()
                        .map(|f| filename_to_basename(f))
                        .to_value()
                }
                "filepath" => {
                    let folder = self.folder.borrow();
                    let filename = self.filename.borrow();
                    match (folder.as_ref(), filename.as_ref()) {
                        (Some(folder), Some(filename)) => {
                            Some(format!("{}/{}", folder, filename)).to_value()
                        }
                        _ => None::<String>.to_value(),
                    }
                }
                _ => unimplemented!(),
            }
        }

        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("deleted")
                        .build(),
                ]
            })
        }
    }
}

glib::wrapper! {
    pub struct Entry(ObjectSubclass<imp::Entry>);
}

impl Entry {
    pub fn new(folder: &str, filename: &str) -> Self {
        glib::Object::builder()
            .property("folder", folder)
            .property("filename", filename)
            .build()
    }

    pub fn read(&self) -> Result<String, String> {
        let filepath = self.property::<Option<String>>("filepath")
            .ok_or("No filepath")?;

        std::fs::read_to_string(&filepath)
            .or_else(|e| {
                if e.kind() == std::io::ErrorKind::NotFound {
                    Ok(String::new())
                } else {
                    Err(format!("Failed to read entry: {}", e))
                }
            })
    }

    pub fn write(&self, text: &str) -> Result<(), String> {
        let filepath = self.property::<Option<String>>("filepath")
            .ok_or("No filepath")?;

        std::fs::write(&filepath, text)
            .map_err(|e| format!("Failed to write entry: {}", e))?;

        println!("saved {}", filepath);
        Ok(())
    }

    pub fn rename_file(&self, new_name: &str) -> Result<(), String> {
        let imp = self.imp();
        let folder = imp.folder.borrow().clone().ok_or("No folder")?;
        let old_filename = imp.filename.borrow().clone().ok_or("No filename")?;

        let old_basename = filename_to_basename(&old_filename);
        let new_basename = filename_to_basename(new_name);

        // Move md file
        let old_file_path = format!("{}/{}", folder, old_filename);
        let new_file_path = format!("{}/{}", folder, new_name);

        std::fs::rename(&old_file_path, &new_file_path)
            .map_err(|e| format!("Failed to rename diary file '{}': {}", old_basename, e))?;

        // Update filename property
        self.set_property("filename", new_name);

        // Move photos folder
        let old_photos_path = format!("{}/photos/{}", folder, old_basename);
        let new_photos_path = format!("{}/photos/{}", folder, new_basename);

        match std::fs::rename(&old_photos_path, &new_photos_path) {
            Ok(_) => {
                // Update photo links in the file
                let old_photos_short = format!("photos/{}", old_basename);
                let new_photos_short = format!("photos/{}", new_basename);

                let input_text = self.read()?;
                let output_text = input_text.replace(&old_photos_short, &new_photos_short);
                self.write(&output_text)?;
            }
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                // Photos folder doesn't exist, that's okay
            }
            Err(e) => {
                return Err(format!("Failed to rename photos folder for diary '{}': {}", old_basename, e));
            }
        }

        Ok(())
    }

    pub fn delete(&self) -> Result<(), String> {
        let filepath = self.property::<Option<String>>("filepath")
            .ok_or("No filepath")?;
        let folder = self.imp().folder.borrow().clone().ok_or("No folder")?;
        let filename = self.imp().filename.borrow().clone().ok_or("No filename")?;

        let basename = filename_to_basename(&filename);

        // Delete entry file
        println!("deleting diary entry {}", filepath);
        std::fs::remove_file(&filepath)
            .map_err(|e| format!("Failed to remove file '{}': {}", filepath, e))?;

        // Delete photos folder
        let photos_path = format!("{}/photos/{}", folder, basename);
        println!("deleting photos in '{}'", photos_path);

        if let Ok(entries) = std::fs::read_dir(&photos_path) {
            for entry in entries.flatten() {
                let path = entry.path();
                if let Err(e) = std::fs::remove_file(&path) {
                    eprintln!("Failed to remove file '{}': {}", path.display(), e);
                }
            }
        }

        match std::fs::remove_dir(&photos_path) {
            Ok(_) => {}
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {}
            Err(e) => {
                eprintln!("Failed to delete photos folder: {}", e);
            }
        }

        self.emit_by_name::<()>("deleted", &[]);

        Ok(())
    }
}

fn filename_to_basename(filename: &str) -> String {
    Path::new(filename)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or(filename)
        .to_string()
}
