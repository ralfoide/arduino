use esp_idf_hal::delay::FreeRtos;
use esp_idf_sys::camera;
use crate::board::Board;
use crate::espcam::Camera;

pub fn run_camera_loop(board: &mut Board) -> anyhow::Result<()> {
    let mut led = board.take_led()?;

    // let camera = board.take_camera()?;
    //
    //
    loop {
    //     let framebuffer = camera.get_framebuffer();
    //
    //     if let Some(framebuffer) = framebuffer {
    //         log::info!("@@ Got framebuffer!");
    //         log::info!("width: {}", framebuffer.width());
    //         log::info!("height: {}", framebuffer.height());
    //         log::info!("len: {}", framebuffer.data().len());
    //         log::info!("format: {}", framebuffer.format());
    //     } else {
    //         log::info!("@@ no framebuffer");
    //     }

        led.set_high()?;
        // we are sleeping here to make sure the watchdog isn't triggered
        FreeRtos::delay_ms(1000);

        led.set_low()?;
        FreeRtos::delay_ms(1000);
        log::info!("---loop---!");
    }
}
