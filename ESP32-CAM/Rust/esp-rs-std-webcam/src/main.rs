mod espcam;

use esp_idf_hal::delay::FreeRtos;
use esp_idf_hal::gpio::*;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::{camera, heap_caps_get_free_size, MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL, MALLOC_CAP_SPIRAM};
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

    let peripherals = Peripherals::take()?;
    let mut led = PinDriver::output(peripherals.pins.gpio33)?;

    let camera = Camera::new(
        peripherals.pins.gpio32,                                // pin_pwdn
        peripherals.pins.gpio0,                                 // pin_xclk
        peripherals.pins.gpio5,                                 // pin_d0
        peripherals.pins.gpio18,                                // pin_d1
        peripherals.pins.gpio19,                                // pin_d2
        peripherals.pins.gpio21,                                // pin_d3
        peripherals.pins.gpio36,                                // pin_d4
        peripherals.pins.gpio39,                                // pin_d5
        peripherals.pins.gpio34,                                // pin_d6
        peripherals.pins.gpio35,                                // pin_d7
        peripherals.pins.gpio25,                                // pin_vsync
        peripherals.pins.gpio23,                                // pin_href
        peripherals.pins.gpio22,                                // pin_pclk
        peripherals.pins.gpio26,                                // pin_sda
        peripherals.pins.gpio27,                                // pin_scl
        // camera::pixformat_t_PIXFORMAT_JPEG,        // pixel_format
        camera::pixformat_t_PIXFORMAT_GRAYSCALE,        // pixel_format
        camera::framesize_t_FRAMESIZE_VGA,        // frame_size
    )
    .unwrap();

    loop {
        let framebuffer = camera.get_framebuffer();

        if let Some(framebuffer) = framebuffer {
            log::info!("@@ Got framebuffer!");
            log::info!("width: {}", framebuffer.width());
            log::info!("height: {}", framebuffer.height());
            log::info!("len: {}", framebuffer.data().len());
            log::info!("format: {}", framebuffer.format());
        } else {
            log::info!("@@ no framebuffer");
        }

        led.set_high()?;
        // we are sleeping here to make sure the watchdog isn't triggered
        FreeRtos::delay_ms(1000);

        led.set_low()?;
        FreeRtos::delay_ms(1000);
        log::info!("---loop---!");
    }
}
