use esp_idf_hal::gpio::{AnyIOPin, AnyOutputPin, Gpio33, Gpio4, Output, OutputPin, PinDriver};
use esp_idf_hal::peripheral::{Peripheral, PeripheralRef};
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_sys::EspError;

pub struct Board2<'a> {
    pub led: PinDriver<'a, AnyOutputPin, Output>,
    pub flash: PinDriver<'a, AnyOutputPin, Output>,
}

impl Board2<'_> {
    pub fn init<'a>() -> Board2<'a> {
        let peripherals = Peripherals::take().unwrap();

        Board2 {
            led: PinDriver::output(peripherals.pins.gpio33.downgrade_output()).unwrap(),
            flash: PinDriver::output(peripherals.pins.gpio4.downgrade_output()).unwrap(),
        }
    }

    // pub fn take_led<'a>(&'a mut self) -> &mut PinDriver<'a, AnyOutputPin, Output> {
    //     &mut self.pin33
    // }
    // 
    // pub fn take_flash<'a>(&mut self) -> &mut PinDriver<'a, AnyOutputPin, Output> {
    //     &mut self.pin04
    // }
}

fn sample_board2() {
    let mut board = Board2::init();

    // let led = board.take_led();
    // let flash = board.take_flash();

    let led = board.led;
    let flash = board.flash;

}

