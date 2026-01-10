use gtk4::glib;
use std::sync::OnceLock;
use std::path::PathBuf;

#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ColorScheme {
    Light = 0,
    Default = 1,
    Dark = 2,
}

use std::sync::Mutex;

#[derive(Debug, Clone)]
struct Settings {
    diary_folder: String,
    color_scheme: ColorScheme,
    config_path: PathBuf,
}

static SETTINGS: OnceLock<Mutex<Settings>> = OnceLock::new();

fn config_dir() -> PathBuf {
    let config_dir = glib::user_config_dir();
    config_dir.join("simple-diary")
}

fn default_data_dir() -> PathBuf {
    let data_dir = glib::user_data_dir();
    data_dir.join("SimpleDiary")
}

pub fn get_diary_folder() -> Result<String, String> {
    let settings = SETTINGS.get().ok_or("Settings not initialized")?;
    let settings = settings.lock().map_err(|_| "Failed to lock settings")?;
    Ok(settings.diary_folder.clone())
}

pub fn get_color_scheme() -> ColorScheme {
    match SETTINGS.get() {
        Some(settings) => {
            match settings.lock() {
                Ok(s) => s.color_scheme,
                Err(_) => {
                    eprintln!("Failed to lock settings");
                    ColorScheme::Default
                }
            }
        }
        None => {
            eprintln!("Settings not initialized, returning default color scheme");
            ColorScheme::Default
        }
    }
}

pub fn set_color_scheme(color_scheme: ColorScheme) -> Result<(), String> {
    let settings_lock = SETTINGS.get().ok_or("Settings not initialized")?;
    let settings = settings_lock.lock().map_err(|_| "Failed to lock settings")?;

    let keyfile = glib::KeyFile::new();
    keyfile.load_from_file(&settings.config_path, glib::KeyFileFlags::empty())
        .map_err(|e| format!("Failed to load config file: {}", e))?;

    let color_scheme_str = match color_scheme {
        ColorScheme::Light => "light",
        ColorScheme::Default => "default",
        ColorScheme::Dark => "dark",
    };

    println!("Saving color scheme preference: {}", color_scheme_str);

    keyfile.set_string("Appearance", "ColorScheme", color_scheme_str);

    keyfile.save_to_file(&settings.config_path)
        .map_err(|e| format!("Error saving settings file: {}", e))?;

    // Update cached value
    drop(settings); // Release lock to get mutable
    let mut settings = settings_lock.lock().map_err(|_| "Failed to lock settings")?;
    settings.color_scheme = color_scheme;

    Ok(())
}

fn load_default_config(keyfile: &glib::KeyFile) -> bool {
    let mut write = false;

    // Notebooks.Default
    if keyfile.string("Notebooks", "Default").is_err() {
        let diary_folder = default_data_dir();
        keyfile.set_string("Notebooks", "Default", &diary_folder.to_string_lossy());
        write = true;
    }

    // Appearance.DarkMode (legacy)
    if keyfile.boolean("Appearance", "DarkMode").is_err() {
        keyfile.set_boolean("Appearance", "DarkMode", false);
        write = true;
    }

    write
}

pub fn init() -> Result<(), String> {
    let config_folder = config_dir();
    println!("Config folder is '{}'", config_folder.display());

    std::fs::create_dir_all(&config_folder)
        .map_err(|e| format!("Failed to create config folder: {}", e))?;

    let keyfile = glib::KeyFile::new();
    let keyfile_path = config_folder.join("settings.ini");

    // Try to load existing keyfile
    if let Err(e) = keyfile.load_from_file(&keyfile_path, glib::KeyFileFlags::empty()) {
        if e.kind::<glib::FileError>() == Some(glib::FileError::Noent) {
            println!("Writing new config");
        } else {
            return Err(format!("Failed to load config: {}", e));
        }
    }

    // Load defaults if needed
    if load_default_config(&keyfile) {
        println!("Writing config at '{}'", keyfile_path.display());
        keyfile.save_to_file(&keyfile_path)
            .map_err(|e| format!("Error saving settings file: {}", e))?;
    }

    // Parse and cache the settings
    let diary_folder = keyfile.string("Notebooks", "Default")
        .map(|s| s.to_string())
        .map_err(|e| format!("Settings file is corrupted: {}", e))?;

    let color_scheme_str = keyfile.string("Appearance", "ColorScheme")
        .unwrap_or_else(|_| glib::GString::from("default"));

    let color_scheme = match color_scheme_str.as_str() {
        "light" => ColorScheme::Light,
        "dark" => ColorScheme::Dark,
        _ => ColorScheme::Default,
    };

    let settings = Settings {
        diary_folder,
        color_scheme,
        config_path: keyfile_path,
    };

    SETTINGS.set(Mutex::new(settings))
        .map_err(|_| "Settings already initialized")?;

    Ok(())
}
