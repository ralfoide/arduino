use crate::espcam::Camera;
use esp_idf_hal::gpio;
use esp_idf_hal::gpio::{Gpio33, Output, PinDriver};
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::{camera, EspError};

pub struct Board {
    pub pins: gpio::Pins,
}

impl Board {
    pub fn init() -> Board {
        let peripherals = Peripherals::take().unwrap();

        Board {
            pins: peripherals.pins,
        }
    }

    pub fn take_led(&mut self) -> Result<PinDriver<Gpio33, Output>, EspError> {
        PinDriver::output(&mut self.pins.gpio33)
    }

    pub fn take_camera(&mut self) -> Result<Camera, EspError> {
        Camera::new(
            &mut self.pins.gpio32,                                // pin_pwdn
            &mut self.pins.gpio0,                                 // pin_xclk
            &mut self.pins.gpio5,                                 // pin_d0
            &mut self.pins.gpio18,                                // pin_d1
            &mut self.pins.gpio19,                                // pin_d2
            &mut self.pins.gpio21,                                // pin_d3
            &mut self.pins.gpio36,                                // pin_d4
            &mut self.pins.gpio39,                                // pin_d5
            &mut self.pins.gpio34,                                // pin_d6
            &mut self.pins.gpio35,                                // pin_d7
            &mut self.pins.gpio25,                                // pin_vsync
            &mut self.pins.gpio23,                                // pin_href
            &mut self.pins.gpio22,                                // pin_pclk
            &mut self.pins.gpio26,                                // pin_sda
            &mut self.pins.gpio27,                                // pin_scl
            // camera::pixformat_t_PIXFORMAT_JPEG,        // pixel_format
            camera::pixformat_t_PIXFORMAT_GRAYSCALE,        // pixel_format
            camera::framesize_t_FRAMESIZE_VGA,        // frame_size
        )
    }
}
