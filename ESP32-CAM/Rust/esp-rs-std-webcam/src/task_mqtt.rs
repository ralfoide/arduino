use std::ptr;
use std::sync::atomic::Ordering;
use embedded_svc::mqtt::client::QoS;
use esp_idf_hal::cpu;
use esp_idf_hal::delay::FreeRtos;
use esp_idf_svc::eventloop::{EspEventLoop, System};
use esp_idf_svc::mqtt::client::{EspMqttClient, EspMqttConnection, EspMqttEvent, MqttClientConfiguration};
use esp_idf_sys::{uxTaskGetStackHighWaterMark2, EspError};
use crate::board::Board;
use crate::shared_data::SHARED_DATA;
use crate::{shared_data, wifi_info};

const MQTT_CLIENT_ID: &str = "esp-mqtt-demo";

const MQTT_TOPIC: &str = "esp-mqtt/counter";

pub fn run_mqtt() -> anyhow::Result<()> {
    wait_for_wifi_ready()?;

    let mqtt_url = format!("mqtt://{}:{}", wifi_info::MQTT_BROKER_IP, wifi_info::MQTT_PORT);
    log::info!("@@ [MQTT] URL: {}", mqtt_url);

    let mut client = mqtt_create(
        &*mqtt_url,
        wifi_info::MQTT_USERNAME,
        wifi_info::MQTT_PASSWORD)?;

    publish_loop(&mut client)
}

fn wait_for_wifi_ready() -> anyhow::Result<()> {
    log::info!("@@ [MQTT] Waiting for wifi...");

    let (lock, cvar) = &*(SHARED_DATA.wifi_ready);
    let mut condition = lock.lock().unwrap();
    while !*condition {
        condition = cvar.wait(condition).unwrap();
    }

    log::info!("@@ [MQTT] Wifi is ready");
    Ok(())
}

fn mqtt_create(
    mqtt_url: &str,
    mqtt_username: &str,
    mqtt_password: &str,
) -> Result<EspMqttClient<'static>, EspError> {
    log::info!("@@ [MQTT] Creating client for URL: {}", mqtt_url);

    let mqtt_client = EspMqttClient::new_cb(
        mqtt_url,
        &MqttClientConfiguration {
            client_id: Some(MQTT_CLIENT_ID),
            username: Some(mqtt_username),
            password: Some(mqtt_password),
            ..Default::default()
        },
        |event| {
            process_event(event)
        }
    )?;

    log::info!("@@ [MQTT] Created client");

    Ok(mqtt_client)
}

fn process_event(event: EspMqttEvent) {
    log::info!("@@ [MQTT] Event received: {:?}", event.payload());
}

fn publish_loop(client: &mut EspMqttClient) -> anyhow::Result<()> {
    let mut last_counter = -1;
    loop {
        let frame_counter = SHARED_DATA.frame_counter.load(Ordering::Relaxed);
        if frame_counter > last_counter {
            last_counter = frame_counter;

            let payload = format!("{}", frame_counter);
            client.enqueue(MQTT_TOPIC, QoS::AtLeastOnce, /*retain=*/false, payload.as_bytes())
                .inspect_err(|e|
                    log::info!("@@ [MQTT] client.enqueue failed: {}", e))
                .ok();
        }

        let core_id: i32 = cpu::core().into();
        let stack_high_water_mark = unsafe { uxTaskGetStackHighWaterMark2(ptr::null_mut()) };
        log::info!("@@ [MQTT] loop. Core #{} -- {} counter -- stack {} mark",
            core_id,
            frame_counter,
        stack_high_water_mark);
        FreeRtos::delay_ms(1000);
    }
}

