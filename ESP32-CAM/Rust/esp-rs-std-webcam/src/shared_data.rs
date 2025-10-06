use std::sync::atomic::AtomicI32;

pub struct SharedData {
    pub frame_counter: AtomicI32,
}

pub static SHARED_DATA: SharedData = SharedData {
    frame_counter: AtomicI32::new(0),
};

