use std::pin::pin;
use std::ptr;
use esp_idf_hal::cpu;
use esp_idf_hal::cpu::Core;
use esp_idf_hal::delay::FreeRtos;
use esp_idf_hal::task::block_on;
use esp_idf_hal::task::thread::ThreadSpawnConfiguration;
use esp_idf_svc::eventloop::{EspEventLoop, EspSystemEventLoop, System};
use esp_idf_sys::{heap_caps_get_free_size, uxTaskGetStackHighWaterMark2, xTaskGetCurrentTaskHandle, MALLOC_CAP_8BIT, MALLOC_CAP_INTERNAL, MALLOC_CAP_SPIRAM};
use esp_rs_std_webcam::board::Board;
use esp_rs_std_webcam::task_camera::CameraCountEvent;

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
    let stack_high_water_mark = unsafe { uxTaskGetStackHighWaterMark2(ptr::null_mut()) };
    log::info!("@@ Stack High Water Mark: {} words", stack_high_water_mark);
    let task_handle = unsafe { xTaskGetCurrentTaskHandle() };
    log::info!("@@ Task Handle: {:?}", task_handle);

    let sys_loop = EspSystemEventLoop::take()?;
    let sys_loop_cam = sys_loop.clone();
    let sys_loop_wifi = sys_loop.clone();

    Board::init()?;

    // Configure threads to have an affinity on Core 1.
    create_thread("cam_task\0", 4096, 1, Core::Core1)
        .spawn(|| {
            log_task_info("CAM");
            // esp_rs_std_webcam::task_camera::run_camera(Board::get(), sys_loop_cam)
        })?;

    create_thread("led_task\0", 4096, 2, Core::Core1)
        .spawn(|| {
            log_task_info("LED");
            // esp_rs_std_webcam::task_led::run_led(Board::get())
        })?;

    create_thread("wifi_task\0", 4096*2, 3, Core::Core0)
        .spawn(|| {
            log_task_info("WIFI");
            // esp_rs_std_webcam::task_wifi::run_wifi(Board::get(), sys_loop_wifi)
        })?;
    let res = esp_rs_std_webcam::task_wifi::run_wifi(Board::get(), sys_loop_wifi);
    log::info!("@@ [WIFI RETURNED] {:?}", res);

    // Block on an infinite "task" (not a thread) with affinity to Core 0.
    block_on(pin!(task_core0(sys_loop)))
}

/*
    Creates a new thread.
    - name: Must be static, max char 16, and containing a terminating \0 character.
    - priority: 1..24 (higher for higher priority). tskIDLE_PRIORITY is 0 and is not allowed here.
    - core: either Core::Core0 or Core::Core1.
 */
fn create_thread(name: &'static str, stack_size: usize, priority: u8 /* 1..24 */, core: Core) -> std::thread::Builder {
    ThreadSpawnConfiguration {
        name: Some(name.as_bytes()), // name must end with \0
        stack_size,
        priority,
        pin_to_core: Some(core),
        ..Default::default()
    }.set()
     .unwrap();

    std::thread::Builder::new()
        .stack_size(stack_size)
}

fn log_task_info(name: &str) {
    let task_handle = unsafe { xTaskGetCurrentTaskHandle() };
    let stack_high_water_mark = unsafe { uxTaskGetStackHighWaterMark2(task_handle) };
    let core_id: i32 = cpu::core().into();
    log::info!("@@ [{}] Task Handle: {:?} -- Core {} -- Stack high mark: {} bytes",
        name,
        task_handle,
        core_id,
        stack_high_water_mark * 4);
}

async fn task_core0(sys_loop: EspEventLoop<System>) -> anyhow::Result<()> {
    // RM: Note that subscribe_async _forces_ the recv().await call below to be blocking.
    // For a non-blocking version, use subscribe() with a closure callback that emits a signal.
    let mut subscription = sys_loop.subscribe_async::<CameraCountEvent>()?;

    loop {
        let stack_high_water_mark = unsafe { uxTaskGetStackHighWaterMark2(ptr::null_mut()) };
        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task 0.. running on Core #{}", core_id);
        log::info!("@@ [TASK 0] Stack High Water Mark: {} bytes", stack_high_water_mark * 4);

        let event = subscription.recv().await?;
        log::info!("@@ Task 0 event recv: {:?}", event);

        FreeRtos::delay_ms(100);
    }
}

