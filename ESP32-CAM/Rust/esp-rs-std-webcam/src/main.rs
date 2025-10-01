mod espcam;

use esp_idf_hal::delay::FreeRtos;
use esp_idf_hal::gpio::*;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::{camera, heap_caps_get_free_size, MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL, MALLOC_CAP_SPIRAM};
use esp_rs_std_webcam::board::{Board};
use esp_rs_std_webcam::board2::Board2;
use esp_rs_std_webcam::espcam::Camera;

fn main() -> anyhow::Result<()> {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_svc::sys::link_patches();

    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    let mut free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_INTERNAL) };
    log::info!("@@ Free internal RAM: {} bytes", free_ram_bytes);
    free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_8BIT) };
    log::info!("@@ Free 8-bit RAM: {} bytes", free_ram_bytes);
    free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_SPIRAM) };
    log::info!("@@ Free external RAM: {} bytes", free_ram_bytes);

    // let mut board = Board::init();
    let mut board2 = Board2::init();

    esp_rs_std_webcam::mod_sample_cam::run_camera_loop(&mut board2)
}
