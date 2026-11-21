use gtk4::prelude::*;

mod entry;
mod entry_bindings;
mod headerbuttons;
mod headerbuttons_bindings;
mod settings;
mod settings_bindings;
mod utils;
mod utils_bindings;
mod widgets;
mod window;
mod window_bindings;

use window::DiaryWindow;

fn activate(app: &adw::Application) {
    let diary_folder = match settings::get_diary_folder() {
        Ok(folder) => folder,
        Err(e) => {
            eprintln!("Failed to get diary folder: {}", e);
            return;
        }
    };
    println!("Diary folder is '{}'", diary_folder);

    if let Err(e) = std::fs::create_dir_all(&diary_folder) {
        eprintln!(
            "Failed to create diary folder at '{}' because:\n{}",
            diary_folder, e
        );
        return;
    }

    utils::apply_color_scheme();

    let window = DiaryWindow::new(app.upcast_ref::<gtk4::Application>());
    window.present();
}

fn main() -> gtk4::glib::ExitCode {
    // Load the GResources bundle compiled by meson (build/res/gresources.gresource)
    let resource_path = concat!(env!("CARGO_MANIFEST_DIR"), "/build/res/gresources.gresource");
    let resource = gtk4::gio::Resource::load(resource_path).expect("failed to load gresources bundle");
    gtk4::gio::resources_register(&resource);

    gtk4::init().expect("failed to initialize GTK");
    adw::init().expect("failed to initialize libadwaita");

    if let Err(e) = settings::init() {
        eprintln!("Failed to initialize settings: {}", e);
    }

    let app = adw::Application::new(
        Some("com.bjareholt.johan.SimpleDiary"),
        gtk4::gio::ApplicationFlags::default(),
    );
    app.connect_activate(activate);
    app.run()
}
