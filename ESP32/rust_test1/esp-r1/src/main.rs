use std::num::NonZeroI32;
use esp_idf_sys as _; // If using the `binstart` feature of `esp-idf-sys`, always keep this module imported
use esp_idf_hal::delay::{Ets, FreeRtos};
use esp_idf_hal::gpio::*;
use esp_idf_hal::peripheral::Peripheral;
use esp_idf_hal::peripherals::Peripherals;
use esp_idf_hal::cpu::core;
use std::thread;
use embedded_graphics::draw_target::DrawTarget;
use embedded_graphics::Drawable;
use embedded_graphics::mono_font::ascii::FONT_10X20;
use embedded_graphics::mono_font::MonoTextStyle;
use embedded_graphics::pixelcolor::BinaryColor;
use embedded_graphics::prelude::{Dimensions, Point, Primitive};
use embedded_graphics::primitives::{PrimitiveStyleBuilder, Rectangle};
use embedded_graphics::text::Text;
use esp_idf_hal::{gpio, i2c};
use esp_idf_hal::i2c::I2C0;
use esp_idf_hal::prelude::FromValueType;
use esp_idf_sys::EspError;
use ssd1306;
use ssd1306::mode::DisplayConfig;

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

    let data = init_data();

    init_oled(data.oled_i2c, data.oled_rst, data.oled_sda, data.oled_scl).unwrap();


    thread::Builder::new()
        .name(String::from("led25"))
        .spawn( || thread_led(data.led_pin1))
        .unwrap();

    FreeRtos::delay_ms(500);

    thread::Builder::new()
        .name(String::from("led21"))
        .spawn( || thread_led(data.led_pin2))
        .unwrap();

    loop {
        println!("@@ main");
        FreeRtos::delay_ms(2000);
    }
}

struct Data {
    //peripherals: Peripherals,
    oled_i2c: I2C0,
    oled_rst: AnyIOPin,
    oled_sda: AnyIOPin,
    oled_scl: AnyIOPin,

    led_pin1: AnyOutputPin,
    led_pin2: AnyOutputPin,
}

fn init_data() -> Data {
    let per = Peripherals::take().unwrap();
    Data {
        //peripherals: per,
        oled_i2c: per.i2c0,
        oled_rst: per.pins.gpio16.into(),
        oled_sda: per.pins.gpio4.into(),
        oled_scl: per.pins.gpio15.into(),

        led_pin1: per.pins.gpio25.into(),
        led_pin2: per.pins.gpio21.into(),
    }
}

fn init_oled(
    i2c: I2C0,
    pin_rst: impl Peripheral<P = impl InputPin + OutputPin>,
    pin_sda: impl Peripheral<P = impl InputPin + OutputPin>,
    pin_scl: impl Peripheral<P = impl InputPin + OutputPin>)
-> Result<(), EspError> {
    println!("@@ init OLED");

    let driver = i2c::I2cDriver::new(
        i2c,
        pin_sda,
        pin_scl,
        &i2c::I2cConfig::new().baudrate(400.kHz().into()),
    )?;
    let interface = ssd1306::I2CDisplayInterface::new(driver);

    let mut reset = gpio::PinDriver::output(pin_rst)?;
    reset.set_high()?;
    Ets::delay_ms(1 as u32);
    reset.set_low()?;
    Ets::delay_ms(10 as u32);
    reset.set_high()?;

    let mut display = ssd1306::Ssd1306::new(
        interface,
        ssd1306::size::DisplaySize128x64,
        ssd1306::rotation::DisplayRotation::Rotate0,
    ).into_buffered_graphics_mode();

    display.init().map_err(|e| {
        println!("Display error: {:?}", e);
        EspError::from_non_zero(NonZeroI32::new(1).unwrap())
    })?;

    println!("@@ init OLED done");

    draw_display(&mut display).unwrap();

    Ok( () )
}

fn thread_led(led_pin: AnyOutputPin) {
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

fn draw_display<D>(display: &mut D) -> Result<(), D::Error>
where
    D: DrawTarget<Color = BinaryColor> + Dimensions
{
    display.clear(BinaryColor::On)?;

    /*
    let bounds = display.bounding_box();
    let r = Rectangle::new(bounds.top_left, bounds.size)
        .into_styled(PrimitiveStyleBuilder::new()
             .fill_color(BinaryColor::Off)
             .stroke_color(BinaryColor::On)
             .stroke_width(1)
             .build(),
        );
    r.draw(display)?;

    let t = Text::new(
        "Hello World",
        Point::new(10, (bounds.size.height - 10) as i32 / 2),
        MonoTextStyle::new(&FONT_10X20, BinaryColor::On)
    );
    t.draw(display)?;
    */

    Ok( () )
}
