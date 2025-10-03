use esp_idf_hal::cpu;
use esp_idf_hal::delay::FreeRtos;
use crate::board::Board;

pub fn run_camera(board: &Board) {
    let camera_mutex = &board.camera.get().unwrap();
    let camera = camera_mutex.lock().unwrap();

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

        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task cam running on Core #{}", core_id);

        FreeRtos::delay_ms(3000);
    }
}
