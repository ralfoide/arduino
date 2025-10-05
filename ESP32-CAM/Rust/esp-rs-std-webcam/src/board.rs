use crate::espcam::Camera;
use esp_idf_hal::gpio::{AnyOutputPin, Output, OutputPin, PinDriver};
use esp_idf_hal::modem::Modem;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::camera;
use std::sync::{Mutex, OnceLock};
use esp_idf_svc::nvs::{EspDefaultNvsPartition, EspNvsPartition, NvsDefault};

pub struct Board {
    pub led: OnceLock<Mutex< PinDriver<'static, AnyOutputPin, Output> >>,
    pub flash: OnceLock<Mutex< PinDriver<'static, AnyOutputPin, Output> >>,
    pub camera: OnceLock<Mutex< Camera<'static> >>,
    pub modem: OnceLock<Mutex< Modem >>,
    pub nvs: OnceLock<Mutex< EspNvsPartition<NvsDefault> >>,
}

static BOARD: Board = Board {
    led: OnceLock::new(),
    flash: OnceLock::new(),
    camera: OnceLock::new(),
    modem: OnceLock::new(),
    nvs: OnceLock::new(),
};

impl Board {
    pub fn get() -> &'static Board {
        &BOARD
    }

    pub fn init() -> anyhow::Result<()> {
        let peripherals = Peripherals::take()?;

        BOARD.led.set(Mutex::new(
            PinDriver::output(peripherals.pins.gpio33.downgrade_output())?
        )).ok();

        BOARD.flash.set(Mutex::new(
            PinDriver::output(peripherals.pins.gpio4.downgrade_output())?
        )).ok();

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
            camera::pixformat_t_PIXFORMAT_JPEG,                     // pixel_format
            // camera::pixformat_t_PIXFORMAT_GRAYSCALE,             // pixel_format
            camera::framesize_t_FRAMESIZE_VGA,                      // frame_size
            10,                                                     // jpeg_quality 0..63
            3,                                                      // fb_count
        )?;

        BOARD.camera.set(Mutex::new(camera)).ok();

        BOARD.modem.set(Mutex::new(peripherals.modem)).ok();

        let nvs: EspNvsPartition<NvsDefault> = EspDefaultNvsPartition::take()?;
        BOARD.nvs.set(Mutex::new(nvs)).ok();

        Ok(())
    }
}
