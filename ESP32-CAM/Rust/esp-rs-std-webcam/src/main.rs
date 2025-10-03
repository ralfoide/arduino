use std::thread::Builder;
use esp_idf_hal::cpu;
use esp_idf_hal::cpu::Core;
use esp_idf_hal::delay::FreeRtos;
use esp_idf_hal::task::thread::ThreadSpawnConfiguration;
use esp_idf_sys::{heap_caps_get_free_size, MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL, MALLOC_CAP_SPIRAM};
use esp_rs_std_webcam::board::Board;

fn main() -> anyhow::Result<()> {

    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_svc::sys::link_patches();

    // Bind the log crate to the ESP Logging facilities
    esp_idf_svc::log::EspLogger::initialize_default();

    // Print the equivalent of xPortGetCoreID()
    // Main always has an affinity to Core 0.
    let core_id: i32 = cpu::core().into();
    log::info!("@@ Main running on Core #{}", core_id);

    let mut free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_INTERNAL) };
    log::info!("@@ Free internal RAM: {} bytes", free_ram_bytes);
    free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_8BIT) };
    log::info!("@@ Free 8-bit RAM: {} bytes", free_ram_bytes);
    free_ram_bytes = unsafe { heap_caps_get_free_size(MALLOC_CAP_SPIRAM) };
    log::info!("@@ Free external RAM: {} bytes", free_ram_bytes);

    Board::init()?;

    // Configure threads to have an affinity on Core 1.
    create_thread("cam_task\0", 1, Core::Core1)
        .spawn(|| {
            esp_rs_std_webcam::task_camera::run_camera(Board::get())
        })?;

    create_thread("led_task\0", 1, Core::Core1)
        .spawn(|| {
            esp_rs_std_webcam::task_led::run_led(Board::get())
        })?;

    // Block on an infinite "task" (not a thread) with affinity to Core 0.
    task_core0();

    Ok(())
}

/*
    Creates a new thread.
    - name: Must be static, max char 16, and containing a terminating \0 character.
    - priority: 1..24 (higher for higher priority). tskIDLE_PRIORITY is 0 and is not allowed here.
    - core: either Core::Core0 or Core::Core1.
 */
fn create_thread(name: &'static str, priority: u8 /* 1..24 */, core: Core) -> Builder {
    ThreadSpawnConfiguration {
        name: Some(name.as_bytes()), // name must end with \0
        stack_size: 4096,
        priority,
        pin_to_core: Some(core),
        ..Default::default()
    }.set()
     .unwrap();

    std::thread::Builder::new()
}

fn task_core0() {
    loop {
        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task 0.. running on Core #{}", core_id);
        FreeRtos::delay_ms(1000);
    }
}

