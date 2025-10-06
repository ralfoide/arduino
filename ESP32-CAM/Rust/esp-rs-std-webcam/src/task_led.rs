use esp_idf_hal::cpu;
use esp_idf_hal::delay::FreeRtos;
use crate::board::Board;

pub fn run_led(board: &'static Board) {

    let led_mutex = &board.led.get().unwrap();
    let mut led = led_mutex.lock().unwrap();

    loop {
        led.set_high().ok();
        // we are sleeping here to make sure the watchdog isn't triggered
        FreeRtos::delay_ms(1000);

        led.set_low().ok();
        FreeRtos::delay_ms(1000);

        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task led running on Core #{}", core_id);
    }
}
