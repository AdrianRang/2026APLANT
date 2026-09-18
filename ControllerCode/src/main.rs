use std::{io::Write, net::UdpSocket, sync::{Arc, RwLock}, thread::{self, sleep}, time::Duration};

use crossterm::event::{self, Event};

fn main() -> std::io::Result<()>{
    println!("Initializing");

    crossterm::terminal::enable_raw_mode()?;
    
    let socket = UdpSocket::bind("0.0.0.0:0").unwrap();
    let input = Arc::new(RwLock::new(String::new()));
    let input_reciever = Arc::clone(&input);

    let send = Arc::new(RwLock::new(false));

    let input_writer = Arc::clone(&input);
    let send_writer = Arc::clone(&send);
    let _input_thread = thread::spawn(move || {
        loop {
            if *input_writer.read().unwrap() == "exit" && *send_writer.read().unwrap() { break; }
            if event::poll(Duration::from_millis(50)).expect("Poll Error") {
                if let Event::Key(key_event) = event::read().expect("Read Error") {
                    match key_event.code {
                        event::KeyCode::Enter => {
                            if input_writer.read().unwrap().len() == 0 { continue; }
                            // message = input.clone().leak();
                            *send_writer.write().unwrap() = true;
                            // *input_writer.write().unwrap() = String::new();
                            print!("\r\n");
                            // break;
                        },
                        event::KeyCode::Char(c) => {
                            input_writer.write().unwrap().push(c);
                            print!("{}", c);
                            std::io::stdout().flush().expect("flush fail");
                        },
                        event::KeyCode::Backspace => { input_writer.write().unwrap().pop(); print!("\x08 \x08"); std::io::stdout().flush().expect("Error Flushing stdout"); }
                        event::KeyCode::Esc => { *input_writer.write().unwrap() = "exit".to_owned(); *send_writer.write().unwrap() = true; }
                        _ => {}
                    }
                }
            }
        }
    });

    loop {
        // print!("1\n\r");
        let mut message = "ACK";

        if *send.read().unwrap() && input_reciever.read().unwrap().len() > 0 {
            message = input_reciever.read().unwrap().clone().leak();
            *input_reciever.write().unwrap() = String::new();
            *send.write().unwrap() = false;
        }
                // print!("2\n\r");
        
        if message == "exit" { break; }

        socket.send_to(message.as_bytes(), "192.168.4.1:3526")?;
        if message != "ACK" {print!("sent {}\n\r", message)}
        // println!("SENT MESSAGE {}", message);
        // print!("3\n\r");
    
        let mut buffer = [0; 1024];

        let (amt, _) = socket.recv_from(&mut buffer)?;

        let recieved = str::from_utf8(&buffer[..amt]).unwrap_or("Invalid UTF-8 Data");
        if message != "ACK" {print!("recieved {}\n\r", recieved)}

        // print!("5\n\r");
        // println!("Recieved reply {}, from {}", response, src);

        sleep(Duration::from_millis(10));
    }

    crossterm::terminal::disable_raw_mode()?;
    
    Ok(())
}
