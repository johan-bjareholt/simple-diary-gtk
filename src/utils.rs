use adw::prelude::*;
use gtk4::glib;

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
        parent,
        gtk4::gio::Cancellable::NONE,
        |_result| {
            // Empty callback, same as the C version
        },
    );
}
