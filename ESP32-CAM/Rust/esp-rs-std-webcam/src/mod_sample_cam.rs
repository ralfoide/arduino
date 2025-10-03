use esp_idf_hal::cpu;
use esp_idf_hal::delay::FreeRtos;
use esp_idf_sys::camera;
use crate::board::Board;
use crate::espcam::Camera;

pub fn run_camera_loop(board: &mut Board) -> anyhow::Result<()> {

    let mut led = &mut board.led;
    let camera = &board.camera;

    loop {
        let framebuffer = camera.get_framebuffer();
    
        if let Some(framebuffer) = framebuffer {
            let data = framebuffer.data();
            log::info!("@@ Got framebuffer: {}", framebuffer);
            log::info!("   width: {}", framebuffer.width());
            log::info!("   height: {}", framebuffer.height());
            log::info!("   data: {:p}", data);
            log::info!("   len: {}", data.len());
            log::info!("   format: {}", framebuffer.format());
        } else {
            log::info!("@@ no framebuffer");
        }

        led.set_high()?;
        // we are sleeping here to make sure the watchdog isn't triggered
        FreeRtos::delay_ms(1000);

        led.set_low()?;
        FreeRtos::delay_ms(1000);
        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task ..1 running on Core #{}", core_id);
    }
}
