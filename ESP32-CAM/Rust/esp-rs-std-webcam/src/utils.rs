use esp_idf_sys::esp_timer_get_time;

pub fn ms_since_boot() -> i64 {
    unsafe {
        let us_since_boot = esp_timer_get_time();
        us_since_boot / 1000
    }
}


pub struct DisplayTimeElapsedMs<'a> {
    label: &'a str,
    start_ms: i64,
}

impl DisplayTimeElapsedMs<'_> {
    pub fn start<'a>(label: &'a str) -> DisplayTimeElapsedMs<'a> {
        DisplayTimeElapsedMs {
            label,
            start_ms: ms_since_boot(),
        }
    }

    pub fn print(&self) {
        let elapsed_ms = ms_since_boot() - self.start_ms;
        log::info!("@@ [{}] Elapsed: {} ms", self.label, elapsed_ms);
    }
}

