use std::sync::{Condvar, LazyLock, Mutex, OnceLock};
use std::sync::atomic::AtomicI32;

pub struct SharedData {
    pub frame_counter: AtomicI32,
    pub last_jpeg: LazyLock<Mutex< Vec<u8> >>,
    pub wifi_ready: LazyLock<(Mutex<bool>, Condvar)>,
}

pub static SHARED_DATA: SharedData = SharedData {
    frame_counter: AtomicI32::new(0),
    last_jpeg: LazyLock::new(|| Mutex::new(Vec::new()) ),
    wifi_ready: LazyLock::new(|| (Mutex::new(false), Condvar::new()) ),
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

    pub fn provide_last_jpeg(&self, jpeg: Vec<u8>) -> anyhow::Result<()> {
        let mut vec_mutex = self.last_jpeg.lock().unwrap();

        // // extend(drain) should just move the content instead of copying it.
        // vec_mutex.clear();
        // vec_mutex.extend(jpeg.drain(..));

        *vec_mutex = jpeg;

        Ok(())
    }

    pub fn consume_last_jpeg(&self) -> anyhow::Result<Vec<u8>> {
        let mut vec_mutex = self.last_jpeg.lock().unwrap();

        let mut dest_vec = Vec::new();
        dest_vec.extend(vec_mutex.drain(..));

        Ok(dest_vec)
    }

}
