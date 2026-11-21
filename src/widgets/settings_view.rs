use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use adw::prelude::*;

use crate::settings::ColorScheme;

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/settings.ui")]
    pub struct SettingsView {
        #[template_child]
        pub color_scheme_light: TemplateChild<gtk4::ToggleButton>,
        #[template_child]
        pub color_scheme_default: TemplateChild<gtk4::ToggleButton>,
        #[template_child]
        pub color_scheme_dark: TemplateChild<gtk4::ToggleButton>,
        #[template_child]
        pub about_button: TemplateChild<adw::ActionRow>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for SettingsView {
        const NAME: &'static str = "SettingsView";
        type Type = super::SettingsView;
        type ParentType = gtk4::Box;

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for SettingsView {
        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            // Reflect the current setting (connected after, to avoid a spurious toggle)
            match crate::settings::get_color_scheme() {
                ColorScheme::Light => self.color_scheme_light.set_active(true),
                ColorScheme::Default => self.color_scheme_default.set_active(true),
                ColorScheme::Dark => self.color_scheme_dark.set_active(true),
            }

            self.color_scheme_light.connect_toggled(glib::clone!(
                #[weak]
                obj,
                move |button| obj.color_scheme_toggled(button, ColorScheme::Light)
            ));
            self.color_scheme_default.connect_toggled(glib::clone!(
                #[weak]
                obj,
                move |button| obj.color_scheme_toggled(button, ColorScheme::Default)
            ));
            self.color_scheme_dark.connect_toggled(glib::clone!(
                #[weak]
                obj,
                move |button| obj.color_scheme_toggled(button, ColorScheme::Dark)
            ));

            self.about_button.connect_activated(glib::clone!(
                #[weak]
                obj,
                move |_| obj.show_about_dialog()
            ));
        }
    }

    impl WidgetImpl for SettingsView {}
    impl BoxImpl for SettingsView {}
}

glib::wrapper! {
    pub struct SettingsView(ObjectSubclass<imp::SettingsView>)
        @extends gtk4::Box, gtk4::Widget,
        @implements gtk4::ConstraintTarget, gtk4::Buildable, gtk4::Accessible;
}

impl SettingsView {
    pub fn new() -> Self {
        glib::Object::new()
    }

    fn color_scheme_toggled(&self, button: &gtk4::ToggleButton, color_scheme: ColorScheme) {
        if button.is_active() {
            if let Err(e) = crate::settings::set_color_scheme(color_scheme) {
                eprintln!("Failed to set color scheme: {}", e);
            }
            crate::utils::apply_color_scheme();
        }
    }

    fn show_about_dialog(&self) {
        let logo =
            gtk4::gdk::Texture::from_resource("/com/bjareholt/johan/simple-diary/icons/logo.svg");

        #[allow(deprecated)]
        let dialog = gtk4::AboutDialog::builder()
            .program_name("Simple Diary")
            .title("About")
            .logo(&logo)
            .authors(["Johan Bjäreholt"])
            .license_type(gtk4::License::Gpl30)
            .website("https://github.com/johan-bjareholt/simple-diary-gtk")
            .build();

        if let Some(window) = self.root().and_downcast::<gtk4::Window>() {
            dialog.set_transient_for(Some(&window));
        }
        dialog.set_modal(true);
        dialog.present();
    }
}
