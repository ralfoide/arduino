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

impl SharedData {

    pub fn signal_wifi_ready(&self, name: &str) -> anyhow::Result<()> {
        log::info!("@@ [{}] Wifi is ready", name);

        let (lock, cvar) = &*(self.wifi_ready);
        let mut condition = lock.lock().unwrap();
        *condition = true;
        cvar.notify_all();

        Ok(())
    }

    pub fn wait_for_wifi_ready(&self, name: &str) -> anyhow::Result<()> {
        log::info!("@@ [{}] Waiting for wifi...", name);

        let (lock, cvar) = &*(self.wifi_ready);
        let mut condition = lock.lock().unwrap();
        while !*condition {
            condition = cvar.wait(condition).unwrap();
        }

        log::info!("@@ [{}] Wifi is ready", name);
        Ok(())
    }
}
