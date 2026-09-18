use std::{io::Write, net::UdpSocket, thread::sleep, time::Duration};

use crossterm::event::{self, Event};

fn main() -> std::io::Result<()>{
    println!("Initializing");

    crossterm::terminal::enable_raw_mode()?;
    
    let socket = UdpSocket::bind("0.0.0.0:0").unwrap();
    let mut input = String::new();

    loop {
        // print!("1\n\r");
        let mut message = "ACK";

        loop {
            if event::poll(Duration::ZERO)? {
                if let Event::Key(key_event) = event::read()? {
                    match key_event.code {
                        event::KeyCode::Enter => {
                            if input.len() == 0 { continue; }
                            message = input.clone().leak();
                            input = String::new();
                            print!("\r\n");
                            // break;
                        },
                        event::KeyCode::Char(c) => {
                            input.push(c);
                            print!("{}", c);
                            std::io::stdout().flush()?
                        },
                        event::KeyCode::Backspace => { input.pop(); print!("\x08 \x08"); std::io::stdout().flush()?; }
                        event::KeyCode::Esc => { message = "exit"; break; }
                        _ => {}
                    }
                }
            } else { break; }
        }
        // print!("2\n\r");
        
        if message == "exit" { break; }

        socket.send_to(message.as_bytes(), "192.168.4.1:3526")?;
        // println!("SENT MESSAGE {}", message);
        // print!("3\n\r");
    
        let mut buffer = [0; 1024];

        let (amt, _) = socket.recv_from(&mut buffer)?;
        // print!("4\n\r");

        // let _ = str::from_utf8(&buffer[..amt]).unwrap_or("Invalid UTF-8 Data");

        // print!("5\n\r");
        // println!("Recieved reply {}, from {}", response, src);

        // sleep(Duration::from_millis(100));
    }

    crossterm::terminal::disable_raw_mode()?;
    
    Ok(())
}
