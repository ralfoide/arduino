use esp_idf_sys as _; // If using the `binstart` feature of `esp-idf-sys`, always keep this module imported
use esp_idf_hal::delay::FreeRtos;
use esp_idf_hal::gpio::*;
use esp_idf_hal::peripheral::Peripheral;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_hal::cpu::core;
use std::thread;

#[cfg(not(feature = "riscv-ulp-hal"))]
fn has_feature_riscv_ulp_hal() {
    println!("@@ CFG: DOES NOT have_feature_riscv_ulp_hal");
}

#[cfg(feature = "riscv-ulp-hal")]
fn has_feature_riscv_ulp_hal() {
    println!("@@ CFG: has_feature_riscv_ulp_hal.");
}

#[cfg(esp32)]
fn has_feature_esp32() {
    println!("@@ CFG: has_feature_esp32.");
}


fn main() {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_sys::link_patches();

    println!("Hello, world!");
    has_feature_esp32();
    has_feature_riscv_ulp_hal();
    println!("@@ Main running on core {}", i32::from(core()));
    println!("@@ Rust main thread: {:?}", thread::current());

    let peripherals = Peripherals::take().unwrap();
    // let mut led = PinDriver::output(peripherals.pins.gpio25).unwrap();
    // let mut led2 = PinDriver::output(peripherals.pins.gpio21).unwrap();
    //
    // led.set_low().unwrap();
    // led2.set_low().unwrap();
    //
    // loop {
    //     led.set_high().unwrap();
    //     FreeRtos::delay_ms(250);
    //     led.set_low().unwrap();
    //     FreeRtos::delay_ms(1000);
    //
    //     led2.set_high().unwrap();
    //     FreeRtos::delay_ms(250);
    //     led2.set_low().unwrap();
    //     FreeRtos::delay_ms(1000);
    //
    //     println!("blink!");
    // }

    thread::Builder::new()
        .name(String::from("led25"))
        .spawn(move || thread_led(peripherals.pins.gpio25))
        .unwrap();

    FreeRtos::delay_ms(500);

    thread::Builder::new()
        .name(String::from("led21"))
        .spawn(move || thread_led(peripherals.pins.gpio21))
        .unwrap();

    loop {
        println!("@@ main");
        FreeRtos::delay_ms(2000);
    }
}

fn thread_led<T: OutputPin>(led_pin: impl Peripheral<P = T>) {
    let mut led = PinDriver::output(led_pin).unwrap();
    let num = led.pin();

    println!("@@ {} Thread running on core {}", num, i32::from(core()));
    println!("@@ {} Rust main thread: {:?}", num, thread::current());

    led.set_low().unwrap();
    loop {
        led.set_high().unwrap();
        FreeRtos::delay_ms(250);
        led.set_low().unwrap();
        FreeRtos::delay_ms(1000);

        println!("@@ {} blink", num);
    }
}
