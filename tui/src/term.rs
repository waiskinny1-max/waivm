use std::io::{self, Read, Write};
use std::process::Command;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Key {
    Char(char),
    Enter,
    Esc,
    Up,
    Down,
    Left,
    Right,
    PageUp,
    PageDown,
    Home,
    End,
    BackTab,
    Unknown,
}

pub struct TerminalGuard {
    saved_mode: String,
    active: bool,
}

impl TerminalGuard {
    pub fn enter() -> io::Result<Self> {
        let saved_mode = read_stty_mode().unwrap_or_default();
        set_raw_mode()?;
        let mut stdout = io::stdout();
        write!(stdout, "\x1b[?1049h\x1b[?25l\x1b[2J\x1b[H")?;
        stdout.flush()?;
        Ok(Self {
            saved_mode,
            active: true,
        })
    }

    pub fn suspend<F>(&mut self, run: F) -> io::Result<i32>
    where
        F: FnOnce() -> io::Result<i32>,
    {
        self.leave()?;
        let result = run();
        println!();
        println!("press Enter to return to waivm-tui...");
        let mut line = String::new();
        let _ = io::stdin().read_line(&mut line);
        self.reenter()?;
        result
    }

    fn leave(&mut self) -> io::Result<()> {
        if self.active {
            restore_stty_mode(&self.saved_mode)?;
            let mut stdout = io::stdout();
            write!(stdout, "\x1b[?25h\x1b[?1049l")?;
            stdout.flush()?;
            self.active = false;
        }
        Ok(())
    }

    fn reenter(&mut self) -> io::Result<()> {
        if !self.active {
            set_raw_mode()?;
            let mut stdout = io::stdout();
            write!(stdout, "\x1b[?1049h\x1b[?25l\x1b[2J\x1b[H")?;
            stdout.flush()?;
            self.active = true;
        }
        Ok(())
    }
}

impl Drop for TerminalGuard {
    fn drop(&mut self) {
        let _ = self.leave();
    }
}

pub fn read_key() -> io::Result<Option<Key>> {
    let mut buffer = [0_u8; 8];
    let count = io::stdin().read(&mut buffer)?;
    if count == 0 {
        return Ok(None);
    }

    Ok(Some(parse_key(&buffer[..count])))
}

pub fn terminal_size() -> (u16, u16) {
    let output = Command::new("stty").arg("size").output();
    if let Ok(output) = output {
        if output.status.success() {
            let text = String::from_utf8_lossy(&output.stdout);
            let mut parts = text.split_whitespace();
            if let (Some(rows), Some(cols)) = (parts.next(), parts.next()) {
                if let (Ok(rows), Ok(cols)) = (rows.parse::<u16>(), cols.parse::<u16>()) {
                    return (rows.max(20), cols.max(60));
                }
            }
        }
    }

    (30, 100)
}

fn parse_key(bytes: &[u8]) -> Key {
    match bytes {
        [b'\r'] | [b'\n'] => Key::Enter,
        [0x1b] => Key::Esc,
        [0x1b, b'[', b'A', ..] => Key::Up,
        [0x1b, b'[', b'B', ..] => Key::Down,
        [0x1b, b'[', b'C', ..] => Key::Right,
        [0x1b, b'[', b'D', ..] => Key::Left,
        [0x1b, b'[', b'H', ..] => Key::Home,
        [0x1b, b'[', b'F', ..] => Key::End,
        [0x1b, b'[', b'Z', ..] => Key::BackTab,
        [0x1b, b'[', b'5', b'~', ..] => Key::PageUp,
        [0x1b, b'[', b'6', b'~', ..] => Key::PageDown,
        [byte] if byte.is_ascii() && !byte.is_ascii_control() => Key::Char(*byte as char),
        _ => Key::Unknown,
    }
}

fn read_stty_mode() -> io::Result<String> {
    let output = Command::new("stty").arg("-g").output()?;
    if !output.status.success() {
        return Err(io::Error::new(io::ErrorKind::Other, "stty -g failed"));
    }
    Ok(String::from_utf8_lossy(&output.stdout).trim().to_string())
}

fn set_raw_mode() -> io::Result<()> {
    let status = Command::new("stty")
        .args(["raw", "-echo", "min", "0", "time", "1"])
        .status()?;
    if status.success() {
        Ok(())
    } else {
        Err(io::Error::new(io::ErrorKind::Other, "failed to enter raw terminal mode"))
    }
}

fn restore_stty_mode(mode: &str) -> io::Result<()> {
    if mode.is_empty() {
        let _ = Command::new("stty").arg("sane").status();
        return Ok(());
    }

    let status = Command::new("stty").arg(mode).status()?;
    if status.success() {
        Ok(())
    } else {
        Err(io::Error::new(io::ErrorKind::Other, "failed to restore terminal mode"))
    }
}
