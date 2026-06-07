use crate::app::{Action, App};
use std::io::{self, Write};

const RESET: &str = "\x1b[0m";
const BOLD: &str = "\x1b[1m";
const DIM: &str = "\x1b[2m";
const REV: &str = "\x1b[7m";
const CYAN: &str = "\x1b[36m";
const GREEN: &str = "\x1b[32m";
const YELLOW: &str = "\x1b[33m";
const RED: &str = "\x1b[31m";

pub fn draw(app: &App, rows: u16, cols: u16) -> io::Result<()> {
    let mut out = String::new();
    out.push_str("\x1b[2J\x1b[H");

    let cols = cols.max(80);
    let rows = rows.max(24);
    let header_h = 5_u16;
    let footer_h = 2_u16;
    let body_h = rows.saturating_sub(header_h + footer_h + 1).max(10);
    let left_w = (cols / 3).clamp(24, 44);
    let right_w = cols.saturating_sub(left_w + 3).max(30);
    let body_y = header_h + 1;

    header(&mut out, app, cols);
    draw_box(&mut out, body_y, 1, body_h, left_w, " programs ");
    draw_box(&mut out, body_y, left_w + 2, body_h, right_w, " output ");

    draw_programs(&mut out, app, body_y + 1, 2, body_h - 2, left_w - 2);
    draw_output(&mut out, app, body_y + 1, left_w + 3, body_h - 2, right_w - 2);
    footer(&mut out, app, rows, cols);

    if app.show_help {
        draw_help(&mut out, rows, cols);
    }

    let mut stdout = io::stdout();
    stdout.write_all(out.as_bytes())?;
    stdout.flush()
}

fn header(out: &mut String, app: &App, cols: u16) {
    write_at(out, 1, 1, &format!("{BOLD}{CYAN}waivm terminal{RESET}"), cols);
    write_at(
        out,
        2,
        1,
        &format!(
            "root: {}    binary: {}",
            app.root.display(),
            app.waivm.display()
        ),
        cols,
    );

    let mut action_line = String::from("actions: ");
    for action in Action::ALL {
        if action == app.action {
            action_line.push_str(&format!(" {REV} {} {RESET}", action.label()));
        } else {
            action_line.push_str(&format!(" {} ", action.label()));
        }
    }
    action_line.push_str("    debug: x");
    write_at(out, 3, 1, &action_line, cols);
    write_at(out, 4, 1, &format!("status: {}", app.status), cols);
}

fn draw_programs(out: &mut String, app: &App, y: u16, x: u16, h: u16, w: u16) {
    if app.programs.is_empty() {
        write_at(out, y, x, "no programs found", w);
        return;
    }

    for row in 0..h {
        let index = row as usize;
        if let Some(program) = app.programs.get(index) {
            let marker = if index == app.selected { "›" } else { " " };
            let style = if index == app.selected { REV } else { "" };
            let reset = if index == app.selected { RESET } else { "" };
            let extension = program
                .path
                .extension()
                .and_then(|value| value.to_str())
                .unwrap_or("");
            let line = format!("{style}{marker} {:<18} {DIM}.{extension}{RESET}{reset}", program.name);
            write_at(out, y + row, x, &line, w);
        }
    }
}

fn draw_output(out: &mut String, app: &App, y: u16, x: u16, h: u16, w: u16) {
    if app.output.is_empty() {
        write_at(out, y, x, "output is empty", w);
        return;
    }

    let start = app.scroll.min(app.output.len().saturating_sub(1));
    for row in 0..h {
        let index = start + row as usize;
        if let Some(line) = app.output.get(index) {
            let styled = style_output_line(line);
            write_at(out, y + row, x, &styled, w);
        }
    }
}

fn footer(out: &mut String, app: &App, rows: u16, cols: u16) {
    let command = if app.last_command.is_empty() {
        "no command executed yet"
    } else {
        &app.last_command
    };
    write_at(
        out,
        rows - 1,
        1,
        "Enter execute | r run | t trace | v verify | d dis | i info | x debug | c clear | ? help | q quit",
        cols,
    );
    write_at(out, rows, 1, &format!("last: {command}"), cols);
}

fn draw_help(out: &mut String, rows: u16, cols: u16) {
    let w = cols.saturating_sub(12).min(74).max(50);
    let h = 14_u16;
    let y = rows.saturating_sub(h) / 2 + 1;
    let x = cols.saturating_sub(w) / 2 + 1;
    draw_box(out, y, x, h, w, " help ");

    let lines = [
        "waivm-tui is a terminal-native control surface for the VM.",
        "It does not embed React, a browser, or a web UI.",
        "",
        "Up/Down or k/j     select program",
        "Left/Right or [/]  change action",
        "Enter              execute selected action",
        "r/t/v/d/i          run, trace, verify, disassemble, info",
        "x                  temporarily leave TUI and open waivm debug",
        "PageUp/PageDown    scroll output",
        "c                  clear output",
        "?                  close this help panel",
        "q or Esc           quit",
    ];

    for (index, line) in lines.iter().enumerate() {
        let style = if index == 0 { BOLD } else { "" };
        write_at(
            out,
            y + 1 + index as u16,
            x + 2,
            &format!("{style}{line}{RESET}"),
            w - 4,
        );
    }
}

fn draw_box(out: &mut String, y: u16, x: u16, h: u16, w: u16, title: &str) {
    if h < 3 || w < 6 {
        return;
    }

    let inner = w.saturating_sub(2) as usize;
    let top = format!("┌{}┐", "─".repeat(inner));
    let bottom = format!("└{}┘", "─".repeat(inner));
    write_at(out, y, x, &top, w);
    write_at(out, y + h - 1, x, &bottom, w);

    for row in 1..h - 1 {
        write_at(out, y + row, x, "│", 1);
        write_at(out, y + row, x + w - 1, "│", 1);
    }

    let title_text = format!("{BOLD}{title}{RESET}");
    write_at(out, y, x + 2, &title_text, w.saturating_sub(4));
}

fn style_output_line(line: &str) -> String {
    if line.starts_with("$") {
        format!("{CYAN}{line}{RESET}")
    } else if line.starts_with("error:") || line.contains("failed") {
        format!("{RED}{line}{RESET}")
    } else if line.starts_with("exit code") {
        format!("{YELLOW}{line}{RESET}")
    } else if line.starts_with("stdout:") {
        format!("{GREEN}{line}{RESET}")
    } else if line.starts_with("stderr:") {
        format!("{RED}{line}{RESET}")
    } else {
        line.to_string()
    }
}

fn write_at(out: &mut String, row: u16, col: u16, text: &str, width: u16) {
    out.push_str(&format!("\x1b[{row};{col}H"));
    out.push_str(&truncate_ansi_safe(text, width as usize));
    out.push_str(RESET);
}

fn truncate_ansi_safe(text: &str, width: usize) -> String {
    if visible_len(text) <= width {
        return text.to_string();
    }

    let mut result = String::new();
    let mut visible = 0_usize;
    let mut chars = text.chars().peekable();

    while let Some(ch) = chars.next() {
        if ch == '\x1b' {
            result.push(ch);
            for next in chars.by_ref() {
                result.push(next);
                if next == 'm' {
                    break;
                }
            }
            continue;
        }

        if visible + 1 >= width {
            result.push('…');
            break;
        }

        result.push(ch);
        visible += 1;
    }

    result
}

fn visible_len(text: &str) -> usize {
    let mut count = 0_usize;
    let mut in_escape = false;

    for ch in text.chars() {
        if ch == '\x1b' {
            in_escape = true;
            continue;
        }
        if in_escape {
            if ch == 'm' {
                in_escape = false;
            }
            continue;
        }
        count += 1;
    }

    count
}
