use std::sync::Arc;
use std::sync::atomic::{AtomicI32, Ordering};
use esp_idf_hal::cpu;
use esp_idf_hal::delay::FreeRtos;
use esp_idf_svc::eventloop::{EspEventLoop, System};
use esp_idf_svc::http::Method;
use esp_idf_svc::http::server::{EspHttpConnection, EspHttpServer, Request};
use esp_idf_svc::wifi::{AuthMethod, BlockingWifi, ClientConfiguration, Configuration, EspWifi};
use crate::board::Board;
use crate::wifi_info;

const HTTPD_STACK_SIZE: usize = 4096*4; // minimum is 6656


pub fn run_wifi(board: &'static Board, sys_loop: EspEventLoop<System>) -> anyhow::Result<()> {
    let modem_mutex = &board.modem.get().unwrap();
    let modem = modem_mutex.lock().unwrap();

    let nvs_mutex = &board.nvs.get().unwrap();
    let nvs = nvs_mutex.lock().unwrap();

    let mut wifi = BlockingWifi::wrap(
        EspWifi::new(modem, sys_loop.clone(), Some(nvs.clone()))?,
        sys_loop,
    )?;

    connect_wifi(&mut wifi)?;

    let mut server = create_server()?;

    let req_count = Arc::new(AtomicI32::new(0));
    let req_count_handler = req_count.clone();

    server.fn_handler("/", Method::Get, move |req| {
        handle_req(req, &req_count_handler)
    })?;

    // This task runner must never terminate, to ensure that the wifi & server are never released.
    // In a real app, this could wait for events on the system event loop.
    loop {
        let core_id: i32 = cpu::core().into();
        let counter = req_count.load(Ordering::Relaxed);
        log::info!("@@ Task WIFI loop.. running on Core #{}... {} requests", core_id, counter);
        FreeRtos::delay_ms(1000);
    }
}

fn handle_req(req: Request<&mut EspHttpConnection>, req_count: &Arc<AtomicI32>) -> anyhow::Result<()> {
    req_count.fetch_add(1, Ordering::Relaxed);
    req.into_ok_response()?
        .write("<html><body>esp-rs</body></html>".as_bytes()).ok();
    Ok(())
}

fn connect_wifi(wifi: &mut BlockingWifi<EspWifi<'static>>) -> anyhow::Result<()> {
    log::info!("@@ [WIFI] Configure, then wait for connection");

    // For the AP (adhoc wifi) case:
    // let wifi_configuration = wifi::Configuration::AccessPoint(AccessPointConfiguration {
    //     ssid: SSID.try_into().unwrap(),
    //     ssid_hidden: true,
    //     auth_method: AuthMethod::WPA2Personal,
    //     password: PASSWORD.try_into().unwrap(),
    //     channel: CHANNEL,
    //     ..Default::default()
    // });

    // To join an existing Access Point:
    let wifi_configuration: Configuration = Configuration::Client(ClientConfiguration {
        ssid: wifi_info::WIFI_SSID.try_into().unwrap(),
        bssid: None,
        auth_method: AuthMethod::WPA2Personal,
        password: wifi_info::WIFI_PASSWD.try_into().unwrap(),
        channel: None,
        ..Default::default()
    });

    wifi.set_configuration(&wifi_configuration)?;

    wifi.start()?;
    log::info!("@@ [WIFI] Connected to {}, waiting for netif up", wifi_info::WIFI_SSID);

    // Note: wait_netif_up has a 15 seconds timeout.
    loop {
        let res = wifi.wait_netif_up();
        if let Ok(_value) = res {
            let ip = wifi.wifi().sta_netif().get_ip_info();
            log::info!("@@ [WIFI] netif UP OK, IP: {:?}", ip);
            break;
        } else {
            log::info!("@@ [WIFI] netif NOT UP: {:?}", res);
        }
    }

    Ok(())
}

fn create_server() -> anyhow::Result< EspHttpServer<'static> > {
    let server_configuration = esp_idf_svc::http::server::Configuration {
        stack_size: HTTPD_STACK_SIZE,
        ..Default::default()
    };

    Ok(EspHttpServer::new(&server_configuration)?)
}

