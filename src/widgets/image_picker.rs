use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::RefCell;

// Module for the ImagePicker implementation
mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/image_picker.ui")]
    pub struct ImagePicker {
        #[template_child]
        pub add_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub cancel_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub name_entry: TemplateChild<gtk4::Entry>,
        #[template_child]
        pub file_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub clipboard_button: TemplateChild<gtk4::Button>,
        #[template_child]
        pub image_preview: TemplateChild<gtk4::Picture>,

        pub basename: RefCell<Option<String>>,
        pub image_texture: RefCell<Option<gtk4::gdk::Texture>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for ImagePicker {
        const NAME: &'static str = "ImagePicker";
        type Type = super::ImagePicker;
        type ParentType = gtk4::Window;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();

            // Install action signals
            klass.install_action("picker.add-image", None, move |obj, _, _| {
                obj.handle_add();
            });

            klass.install_action("picker.cancel", None, move |obj, _, _| {
                obj.handle_cancel();
            });
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for ImagePicker {
        fn signals() -> &'static [glib::subclass::Signal] {
            use std::sync::OnceLock;
            static SIGNALS: OnceLock<Vec<glib::subclass::Signal>> = OnceLock::new();
            SIGNALS.get_or_init(|| {
                vec![
                    glib::subclass::Signal::builder("image-selected")
                        .param_types([String::static_type(), String::static_type()])
                        .build(),
                    glib::subclass::Signal::builder("cancelled")
                        .build(),
                ]
            })
        }

        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            // Connect file and clipboard buttons
            self.file_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.select_file()
            ));

            self.clipboard_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| obj.select_clipboard()
            ));

            // Connect add button to action
            self.add_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "picker.add-image", None).unwrap()
            ));

            // Connect cancel button to action
            self.cancel_button.connect_clicked(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "picker.cancel", None).unwrap()
            ));

            // Connect entry activate to add action
            self.name_entry.connect_activate(glib::clone!(
                #[weak] obj,
                move |_| WidgetExt::activate_action(&obj, "picker.add-image", None).unwrap()
            ));
        }
    }

    impl WidgetImpl for ImagePicker {}
    impl WindowImpl for ImagePicker {}
}

glib::wrapper! {
    pub struct ImagePicker(ObjectSubclass<imp::ImagePicker>)
        @extends gtk4::Window, gtk4::Widget,
        @implements gtk4::gio::ActionGroup, gtk4::gio::ActionMap;
}

impl ImagePicker {
    pub fn new(basename: &str) -> Self {
        let obj: Self = glib::Object::new();
        obj.imp().basename.replace(Some(basename.to_string()));
        obj
    }

    fn select_file(&self) {
        let dialog = gtk4::FileDialog::new();
        let self_weak = self.downgrade();

        dialog.open(
            Some(self),
            gtk4::gio::Cancellable::NONE,
            move |result| {
                let Some(this) = self_weak.upgrade() else { return };

                match result {
                    Ok(file) => {
                        if let Err(e) = this.load_image_from_file(&file) {
                            crate::utils::error_dialog(&this, &format!("Failed to load image: {}", e));
                        }
                    }
                    Err(e) => {
                        println!("Failed to select file: {}", e);
                    }
                }
            },
        );
    }

    fn load_image_from_file(&self, file: &gtk4::gio::File) -> Result<(), String> {
        let path = file.path().ok_or("Failed to get file path")?;
        let pixbuf = gtk4::gdk_pixbuf::Pixbuf::from_file(&path)
            .map_err(|e| e.to_string())?;

        #[allow(deprecated)]
        let texture = gtk4::gdk::Texture::for_pixbuf(&pixbuf);

        self.imp().image_texture.replace(Some(texture.clone()));
        self.imp().image_preview.set_paintable(Some(&texture));

        Ok(())
    }

    fn select_clipboard(&self) {
        let clipboard = self.clipboard();
        let self_weak = self.downgrade();

        clipboard.read_texture_async(
            gtk4::gio::Cancellable::NONE,
            move |result| {
                let Some(this) = self_weak.upgrade() else { return };

                match result {
                    Ok(Some(texture)) => {
                        this.imp().image_texture.replace(Some(texture.clone()));
                        this.imp().image_preview.set_paintable(Some(&texture));
                    }
                    Ok(None) => {
                        crate::utils::error_dialog(&this, "No texture from clipboard");
                    }
                    Err(e) => {
                        crate::utils::error_dialog(&this, &format!("Failed to load from clipboard: {}", e));
                    }
                }
            },
        );
    }

    fn save_image(&self) -> Option<(String, String)> {
        let imp = self.imp();

        let basename = imp.basename.borrow().as_ref()?.clone();
        let texture = imp.image_texture.borrow().as_ref()?.clone();
        let image_name = imp.name_entry.text().to_string();

        if image_name.is_empty() {
            crate::utils::error_dialog(self, "Please enter an image name");
            return None;
        }

        // Get paths
        let window = self.upcast_ref::<gtk4::Window>();
        let target_dir_relative = crate::utils::get_photos_folder(&basename, false, Some(window))?;
        let target_dir_absolute = crate::utils::get_photos_folder(&basename, true, Some(window))?;

        let image_path_relative = format!("{}/{}.png", target_dir_relative, image_name);
        let image_path_absolute = format!("{}/{}.png", target_dir_absolute, image_name);

        // Save texture
        if let Err(e) = texture.save_to_png(&image_path_absolute) {
            crate::utils::error_dialog(self, &format!("Could not save image: {}", e));
            return None;
        }

        Some((image_name, image_path_relative))
    }

    fn handle_add(&self) {
        if let Some((name, path)) = self.save_image() {
            self.emit_by_name::<()>("image-selected", &[&name, &path]);
            self.close();
        }
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
