// Source: https://github.com/esp-rs/esp-idf-svc/blob/master/examples/http_server.rs
// Source: https://github.com/esp-rs/esp-idf-svc/blob/master/examples/http_server_page.html
//
//! HTTP Server with JSON POST handler
//!
//! Go to 192.168.71.1 to test

use core::convert::TryInto;

use embedded_svc::{
    http::{Headers, Method},
    io::{Read, Write},
    wifi::{self, AccessPointConfiguration, AuthMethod},
};
use embedded_svc::wifi::ClientConfiguration;
use esp_idf_svc::hal::peripherals::Peripherals;
use esp_idf_svc::{
    eventloop::EspSystemEventLoop,
    http::server::EspHttpServer,
    nvs::EspDefaultNvsPartition,
    wifi::{BlockingWifi, EspWifi},
};
use esp_idf_svc::sys::{esp, esp_wifi_set_bandwidth, wifi_bandwidth_t_WIFI_BW_HT20, wifi_interface_t_WIFI_IF_AP};
use log::*;

use serde::Deserialize;

// For testing wifi purposes only:
// const AP_SSID: &str = env!("WIFI_SSID");
// const AP_PASSWORD: &str = env!("WIFI_PASS");
const AP_SSID: &str = "esp32_wifi";
const AP_PASSWORD: &str = "esp32_pass";

const WIFI_IS_AP: bool = false;


static INDEX_HTML: &str = include_str!("http_server_page.html");

// Max payload length
const MAX_LEN: usize = 128;

// Need lots of stack to parse JSON
const STACK_SIZE: usize = 10240;

// Wi-Fi channel, between 1 and 11
const AP_CHANNEL: u8 = 5;

#[derive(Deserialize)]
struct FormData<'a> {
    first_name: &'a str,
    age: u32,
    birthplace: &'a str,
}

fn main() -> anyhow::Result<()> {
    esp_idf_svc::sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    // Setup Wifi

    let peripherals = Peripherals::take()?;
    let sys_loop = EspSystemEventLoop::take()?;
    let nvs = EspDefaultNvsPartition::take()?;

    let mut wifi = BlockingWifi::wrap(
        EspWifi::new(peripherals.modem, sys_loop.clone(), Some(nvs))?,
        sys_loop,
    )?;

    connect_wifi(&mut wifi)?;

    let mut server = create_server()?;

    server.fn_handler("/", Method::Get, |req| {
        req.into_ok_response()?
            .write_all(INDEX_HTML.as_bytes())
            .map(|_| ())
    })?;

    server.fn_handler::<anyhow::Error, _>("/post", Method::Post, |mut req| {
        let len = req.content_len().unwrap_or(0) as usize;

        if len > MAX_LEN {
            req.into_status_response(413)?
                .write_all("Request too big".as_bytes())?;
            return Ok(());
        }

        let mut buf = vec![0; len];
        req.read_exact(&mut buf)?;
        let mut resp = req.into_ok_response()?;

        if let Ok(form) = serde_json::from_slice::<FormData>(&buf) {
            write!(
                resp,
                "Hello, {}-year-old {} from {}!",
                form.age, form.first_name, form.birthplace
            )?;
        } else {
            resp.write_all("JSON error".as_bytes())?;
        }

        Ok(())
    })?;

    // Keep wifi and the server running beyond when main() returns (forever)
    // Do not call this if you ever want to stop or access them later.
    // Otherwise you can either add an infinite loop so the main task
    // never returns, or you can move them to another thread.
    // https://doc.rust-lang.org/stable/core/mem/fn.forget.html
    core::mem::forget(wifi);
    core::mem::forget(server);

    // Main task no longer needed, free up some memory
    Ok(())
}

fn connect_wifi(wifi: &mut BlockingWifi<EspWifi<'static>>) -> anyhow::Result<()> {
    // If instead of creating a new network you want to serve the page
    // on your local network, you can replace this configuration with
    // the client configuration from the http_client example.

    let wifi_configuration: wifi::Configuration = if WIFI_IS_AP {
        // For the AP (adhoc wifi) case:
        wifi::Configuration::AccessPoint(AccessPointConfiguration {
            ssid: AP_SSID.try_into().unwrap(),
            ssid_hidden: false,
            auth_method: AuthMethod::WPA2Personal,
            password: AP_PASSWORD.try_into().unwrap(),
            channel: AP_CHANNEL,
            ..Default::default()
        })
    } else {
        // To join an existing Access Point:
        wifi::Configuration::Client(ClientConfiguration {
            ssid: esp_rs_std_wifi::wifi_info::WIFI_SSID.try_into().unwrap(),
            bssid: None,
            auth_method: AuthMethod::WPA2Personal,
            password: esp_rs_std_wifi::wifi_info::WIFI_PASSWD.try_into().unwrap(),
            channel: None,
            ..Default::default()
        })
    };

    wifi.set_configuration(&wifi_configuration)?;

    if WIFI_IS_AP {
        // RM switch the AP bandwidth to HT20
        esp!(unsafe { esp_wifi_set_bandwidth(wifi_interface_t_WIFI_IF_AP, wifi_bandwidth_t_WIFI_BW_HT20) })?;
    }

    wifi.start()?;
    info!("Wifi started");

    // If using a client configuration you need
    // to connect to the network with:
    if !WIFI_IS_AP {
         wifi.connect()?;
         info!("Wifi connected");
    }

    wifi.wait_netif_up()?;
    info!("Wifi netif up");

    info!("Created Wi-Fi");

    Ok(())
}

fn create_server() -> anyhow::Result<EspHttpServer<'static>> {
    let server_configuration = esp_idf_svc::http::server::Configuration {
        stack_size: STACK_SIZE,
        ..Default::default()
    };

    Ok(EspHttpServer::new(&server_configuration)?)
}
