use std::ffi::CStr;
use esp_idf_hal::{cpu, delay};
use esp_idf_hal::delay::FreeRtos;
use esp_idf_svc::eventloop::{EspEvent, EspEventDeserializer, EspEventLoop, EspEventPostData, EspEventSerializer, EspEventSource, EspSystemEventLoop, System};
use crate::board::Board;

pub fn run_camera(board: &'static Board, sys_loop: EspEventLoop<System>) -> anyhow::Result<()> {
    let camera_mutex = &board.camera.get().unwrap();
    let camera = camera_mutex.lock().unwrap();

    let mut count_event = CameraCountEvent {
        frame_count: 0,
    };

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

            count_event.frame_count += 1;
            sys_loop.post::<CameraCountEvent>(&count_event, delay::BLOCK)?;

        } else {
            log::info!("@@ no framebuffer");
        }

        let core_id: i32 = cpu::core().into();
        log::info!("@@ Task cam running on Core #{}", core_id);

        FreeRtos::delay_ms(3000);
    }
}

// RM: Note that the ESP-RS wrapper EspEventPostData necessarily treats the payload as a Ptr
// and a length, which is similar to what the ESP-IDF "esp_event_post()" method does.
#[allow(dead_code)]
#[derive(Copy, Clone, Debug)]
pub struct CameraCountEvent {
    pub frame_count: u32,
}

// RM: events are identified by in ESP by an "event base" and here by a "source" (a static CStr)
// then each event in the source has a unique event id.
unsafe impl EspEventSource for CameraCountEvent {
    #[allow(clippy::manual_c_str_literals)]
    fn source() -> Option<&'static CStr> {
        // String should be unique across the whole project and ESP IDF
        Some(CStr::from_bytes_with_nul(b"TASK_CAM\0").unwrap())
    }

    fn event_id() -> Option<i32> {
        Some(1)
    }
}

impl EspEventSerializer for CameraCountEvent {
    type Data<'a> = CameraCountEvent;

    fn serialize<F, R>(event: &Self::Data<'_>, f: F) -> R
    where
        F: FnOnce(&EspEventPostData) -> R
    {
        // This only works if the payload implements Copy and is static.
        // It basically retrieves a void* ptr to the actual event structure.
        f(&unsafe { EspEventPostData::new(Self::source().unwrap(), Self::event_id(), event) })
    }
}

impl EspEventDeserializer for CameraCountEvent {
    type Data<'a> = CameraCountEvent;

    fn deserialize<'a>(data: &EspEvent<'a>) -> Self::Data<'a> {
        // Just as easy as serializing
        *unsafe { data.as_payload::<CameraCountEvent>() }
    }
}


