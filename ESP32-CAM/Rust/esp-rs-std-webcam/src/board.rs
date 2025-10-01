use crate::espcam::Camera;
use esp_idf_hal::gpio::{AnyOutputPin, Output, OutputPin, PinDriver};
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::camera;

pub struct Board {
    pub led: PinDriver<'static, AnyOutputPin, Output>,
    pub flash: PinDriver<'static, AnyOutputPin, Output>,
    pub camera: Camera<'static>,
}

impl Board {
    pub fn init() -> Board {
        let peripherals = Peripherals::take().unwrap();

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
            camera::pixformat_t_PIXFORMAT_GRAYSCALE,                // pixel_format
            camera::framesize_t_FRAMESIZE_VGA,                      // frame_size
            10,                                                     // jpeg_quality 0..63
            2,                                                      // fb_count
        );

        Board {
            led: PinDriver::output(peripherals.pins.gpio33.downgrade_output()).unwrap(),
            flash: PinDriver::output(peripherals.pins.gpio4.downgrade_output()).unwrap(),
            camera: camera.unwrap(),
        }
    }
}

fn sample_board() {
    let mut board = Board::init();

    let _led = board.led;
    let _flash = board.flash;
    let _camera = board.camera;
}

