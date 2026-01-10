use std::ffi::CString;
use std::os::raw::{c_char, c_int};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn settings_init() -> c_int {
    match crate::settings::init() {
        Ok(()) => 0,
        Err(e) => {
            eprintln!("Failed to initialize settings: {}", e);
            std::process::exit(1);
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn settings_get_diary_folder() -> *mut c_char {
    match crate::settings::get_diary_folder() {
        Ok(folder) => {
            match CString::new(folder) {
                Ok(c_str) => c_str.into_raw(),
                Err(_) => {
                    eprintln!("Failed to convert diary folder to C string");
                    std::ptr::null_mut()
                }
            }
        }
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(1);
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn settings_get_color_scheme() -> c_int {
    crate::settings::get_color_scheme() as c_int
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn settings_set_color_scheme(color_scheme: c_int) {
    let scheme = match color_scheme {
        0 => crate::settings::ColorScheme::Light,
        1 => crate::settings::ColorScheme::Default,
        2 => crate::settings::ColorScheme::Dark,
        _ => {
            eprintln!("Invalid color scheme value: {}", color_scheme);
            std::process::exit(1);
        }
    };
    
    if let Err(e) = crate::settings::set_color_scheme(scheme) {
        eprintln!("{}", e);
        std::process::exit(1);
    }
}
