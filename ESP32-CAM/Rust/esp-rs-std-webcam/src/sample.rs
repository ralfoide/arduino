

pub struct ExamplePin {
    pub name: String,
    pub inUse: bool,
}

pub struct ExampleBoard {
    pub pins: Vec<ExamplePin>,
}

impl ExampleBoard {
    pub fn init() -> ExampleBoard {
        ExampleBoard {
            pins: vec![
                ExamplePin {
                    name: "one".to_string(),
                    inUse: false,
                },
                ExamplePin {
                    name: "two".to_string(),
                    inUse: false,
                },
            ],
        }
    }

    pub fn take_led(&mut self) -> Option<&mut ExamplePin> {
        self.pins.push(ExamplePin {
            name: "led".to_string(),
            inUse: false,
        });
        let pin = self.pins.get_mut(2).unwrap();
        pin.inUse = true;
        Some(pin)
    }

    pub fn take_flash(&mut self) -> Option<&mut ExamplePin> {
        let pin = self.pins.get_mut(1).unwrap();
        pin.inUse = true;
        Some(pin)
    }
}

fn sample_main() {
    let mut board = ExampleBoard::init();

    let led = board.take_led().unwrap();
    let flash = board.take_flash().unwrap();

}

