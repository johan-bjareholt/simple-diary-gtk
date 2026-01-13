use adw::prelude::*;
use std::path::Path;

/// Display an error dialog with the given message
///
/// # Arguments
/// * `parent` - The parent widget for the dialog
/// * `message` - The error message to display
pub fn error_dialog<P: IsA<gtk4::Widget>>(parent: &P, message: &str) {
    eprintln!("Error: {}", message);

    let dialog = adw::AlertDialog::new(Some("Error"), Some(message));

    dialog.add_response("close", "_Close");
    dialog.set_response_appearance("close", adw::ResponseAppearance::Destructive);

    dialog.choose(
        Some(parent),
        gtk4::gio::Cancellable::NONE,
        |_result| {
            // Empty callback, same as the C version
        },
    );
}

/// Get the diary folder path
pub fn get_diary_folder() -> Result<String, String> {
    crate::settings::get_diary_folder()
}

/// Get the photos folder for a given entry basename
///
/// # Arguments
/// * `basename` - The entry basename
/// * `absolute` - Whether to return absolute or relative path
/// * `window` - The window for showing error dialogs
pub fn get_photos_folder(
    basename: &str,
    absolute: bool,
    window: Option<&gtk4::Window>,
) -> Option<String> {
    let diary_folder = match get_diary_folder() {
        Ok(folder) => folder,
        Err(e) => {
            eprintln!("Failed to get diary folder: {}", e);
            return None;
        }
    };

    let photos_folder_absolute = format!("{}/photos/{}", diary_folder, basename);
    let photos_path = Path::new(&photos_folder_absolute);

    if let Err(e) = std::fs::create_dir_all(photos_path) {
        let msg = format!(
            "Failed to create folder at '{}' because: {}",
            photos_folder_absolute, e
        );
        if let Some(win) = window {
            error_dialog(win, &msg);
        } else {
            eprintln!("{}", msg);
        }
        return None;
    }

    if absolute {
        Some(photos_folder_absolute)
    } else {
        Some(format!("photos/{}", basename))
    }
}

/// Apply the current color scheme to the application
pub fn apply_color_scheme() {
    let color_scheme = crate::settings::get_color_scheme();

    let app = gtk4::gio::Application::default()
        .and_downcast::<adw::Application>()
        .expect("Failed to get AdwApplication");

    let style_mgr = app.style_manager();

    let adw_scheme = match color_scheme {
        crate::settings::ColorScheme::Light => adw::ColorScheme::PreferLight,
        crate::settings::ColorScheme::Default => adw::ColorScheme::Default,
        crate::settings::ColorScheme::Dark => adw::ColorScheme::PreferDark,
    };

    style_mgr.set_color_scheme(adw_scheme);
}
