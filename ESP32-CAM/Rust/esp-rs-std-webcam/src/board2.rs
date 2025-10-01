use esp_idf_hal::gpio::{Gpio33, Gpio4, Output, PinDriver};
use esp_idf_hal::peripheral::{Peripheral, PeripheralRef};
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::{camera, EspError};

pub struct Board2<'a> {
    pin04: PeripheralRef<'a, Gpio4>,
    pin33: PeripheralRef<'a, Gpio33>,
}

impl Board2<'_> {
    pub fn init<'a>() -> Board2<'a> {
        let peripherals = Peripherals::take().unwrap();

        Board2 {
            pin04: peripherals.pins.gpio4.into_ref(),
            pin33: peripherals.pins.gpio33.into_ref(),
        }
    }

    pub fn take_led(&mut self) -> Result<PinDriver<Gpio33, Output>, EspError> {
        PinDriver::output(&mut self.pin33)
    }

    pub fn take_flash(&mut self) -> Result<PinDriver<Gpio4, Output>, EspError> {
        PinDriver::output(&mut self.pin04)
    }
}
