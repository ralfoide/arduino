use std::sync::{Condvar, LazyLock, Mutex};
use std::sync::atomic::AtomicI32;

pub struct SharedData {
    pub frame_counter: AtomicI32,
    pub wifi_ready: LazyLock<(Mutex<bool>, Condvar)>,
}

pub static SHARED_DATA: SharedData = SharedData {
    frame_counter: AtomicI32::new(0),
    wifi_ready: LazyLock::new(|| { (Mutex::new(false), Condvar::new()) }),
};

