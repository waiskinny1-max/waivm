mod app;
mod runner;
mod term;
mod ui;

use app::{parse_args, usage, Action, App, ParseOutcome};
use std::error::Error;
use std::io;
use term::{Key, TerminalGuard};

fn main() {
    if let Err(err) = run() {
        eprintln!("waivm-tui: {err}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn Error>> {
    let config = match parse_args().map_err(|err| io::Error::new(io::ErrorKind::InvalidInput, err))? {
        ParseOutcome::Help => {
            print!("{}", usage());
            return Ok(());
        }
        ParseOutcome::Run(config) => config,
    };

    let mut app = App::new(config)?;
    let mut terminal = TerminalGuard::enter()?;

    loop {
        let (rows, cols) = term::terminal_size();
        ui::draw(&app, rows, cols)?;

        if app.should_quit {
            break;
        }

        if let Some(key) = term::read_key()? {
            if let Some(debug_file) = handle_key(&mut app, key) {
                let root = app.root.clone();
                let waivm = app.waivm.clone();
                let debug_file_for_runner = debug_file.clone();
                let code = terminal.suspend(|| runner::launch_debugger(&waivm, &root, &debug_file_for_runner))?;
                app.status = format!("debugger exited with code {code}");
                app.output = vec![
                    format!("$ {} debug {}", app.waivm.display(), debug_file.display()),
                    format!("debugger exited with code {code}"),
                ];
            }
        }
    }

    Ok(())
}

fn handle_key(app: &mut App, key: Key) -> Option<std::path::PathBuf> {
    match key {
        Key::Char('q') | Key::Esc => app.should_quit = true,
        Key::Char('?') | Key::Char('h') => app.show_help = !app.show_help,
        Key::Char('j') | Key::Down => app.select_next(),
        Key::Char('k') | Key::Up => app.select_prev(),
        Key::Right | Key::Char(']') => app.action = app.action.next(),
        Key::Left | Key::Char('[') | Key::BackTab => app.action = app.action.prev(),
        Key::PageUp => app.page_up(),
        Key::PageDown => app.page_down(),
        Key::Home => app.scroll = 0,
        Key::End => app.scroll = app.output.len().saturating_sub(1),
        Key::Char('c') => app.clear_output(),
        Key::Char('r') => execute_action(app, Action::Run),
        Key::Char('t') => execute_action(app, Action::Trace),
        Key::Char('v') => execute_action(app, Action::Verify),
        Key::Char('d') => execute_action(app, Action::Disassemble),
        Key::Char('i') => execute_action(app, Action::Info),
        Key::Enter => {
            let action = app.action;
            execute_action(app, action);
        }
        Key::Char('x') => {
            if let Some(path) = app.selected_path() {
                app.status = "launching interactive debugger".to_string();
                return Some(path);
            }
            app.status = "no program selected".to_string();
        }
        Key::Unknown | Key::Char(_) => {}
    }

    None
}

fn execute_action(app: &mut App, action: Action) {
    app.set_action(action);

    let Some(program) = app.selected_program() else {
        app.status = "no program selected".to_string();
        return;
    };

    let program_path = program.path.clone();
    app.last_command = format!(
        "{} {} {}",
        app.waivm.display(),
        action.cli_command(),
        program_path.display()
    );
    app.status = format!("executed {} on {}", action.label(), program.name);

    let lines = runner::execute(&app.waivm, &app.root, action, &program_path);
    app.append_output(lines);
}
