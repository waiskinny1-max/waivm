use std::env;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Action {
    Run,
    Trace,
    Verify,
    Disassemble,
    Info,
}

impl Action {
    pub const ALL: [Action; 5] = [
        Action::Run,
        Action::Trace,
        Action::Verify,
        Action::Disassemble,
        Action::Info,
    ];

    pub fn label(self) -> &'static str {
        match self {
            Action::Run => "run",
            Action::Trace => "trace",
            Action::Verify => "verify",
            Action::Disassemble => "dis",
            Action::Info => "info",
        }
    }

    pub fn cli_command(self) -> &'static str {
        match self {
            Action::Run => "run",
            Action::Trace => "trace",
            Action::Verify => "verify",
            Action::Disassemble => "dis",
            Action::Info => "info",
        }
    }

    pub fn next(self) -> Self {
        let index = Self::ALL.iter().position(|item| *item == self).unwrap_or(0);
        Self::ALL[(index + 1) % Self::ALL.len()]
    }

    pub fn prev(self) -> Self {
        let index = Self::ALL.iter().position(|item| *item == self).unwrap_or(0);
        Self::ALL[(index + Self::ALL.len() - 1) % Self::ALL.len()]
    }
}

#[derive(Clone, Debug)]
pub struct Program {
    pub name: String,
    pub path: PathBuf,
}

#[derive(Debug)]
pub struct App {
    pub root: PathBuf,
    pub waivm: PathBuf,
    pub programs: Vec<Program>,
    pub selected: usize,
    pub action: Action,
    pub output: Vec<String>,
    pub status: String,
    pub last_command: String,
    pub should_quit: bool,
    pub show_help: bool,
    pub scroll: usize,
}

impl App {
    pub fn new(config: Config) -> io::Result<Self> {
        let root = detect_root(config.root)?;
        let waivm = detect_waivm(&root, config.waivm);
        let mut programs = collect_programs(&root)?;

        if let Some(file) = config.file {
            let path = if file.is_absolute() { file } else { root.join(file) };
            let name = path
                .file_name()
                .and_then(|value| value.to_str())
                .unwrap_or("custom")
                .to_string();
            programs.insert(0, Program { name, path });
        }

        let mut output = Vec::new();
        output.push("waivm terminal UI ready.".to_string());
        output.push("Press Enter to execute the selected command, ? for help, q to quit.".to_string());

        let status = if programs.is_empty() {
            "No .wai or .waibc programs found. Use --file <path> or add examples.".to_string()
        } else {
            format!("Loaded {} program(s).", programs.len())
        };

        Ok(Self {
            root,
            waivm,
            programs,
            selected: 0,
            action: Action::Run,
            output,
            status,
            last_command: String::new(),
            should_quit: false,
            show_help: false,
            scroll: 0,
        })
    }

    pub fn selected_program(&self) -> Option<&Program> {
        self.programs.get(self.selected)
    }

    pub fn selected_path(&self) -> Option<PathBuf> {
        self.selected_program().map(|program| program.path.clone())
    }

    pub fn select_next(&mut self) {
        if !self.programs.is_empty() {
            self.selected = (self.selected + 1) % self.programs.len();
        }
    }

    pub fn select_prev(&mut self) {
        if !self.programs.is_empty() {
            self.selected = (self.selected + self.programs.len() - 1) % self.programs.len();
        }
    }

    pub fn set_action(&mut self, action: Action) {
        self.action = action;
    }

    pub fn clear_output(&mut self) {
        self.output.clear();
        self.scroll = 0;
        self.status = "Output cleared.".to_string();
    }

    pub fn append_output(&mut self, lines: Vec<String>) {
        self.output = lines;
        self.scroll = 0;
    }

    pub fn scroll_up(&mut self) {
        self.scroll = self.scroll.saturating_sub(1);
    }

    pub fn scroll_down(&mut self) {
        if self.scroll + 1 < self.output.len() {
            self.scroll += 1;
        }
    }

    pub fn page_up(&mut self) {
        self.scroll = self.scroll.saturating_sub(10);
    }

    pub fn page_down(&mut self) {
        self.scroll = (self.scroll + 10).min(self.output.len().saturating_sub(1));
    }
}

#[derive(Debug, Default)]
pub struct Config {
    pub root: Option<PathBuf>,
    pub waivm: Option<PathBuf>,
    pub file: Option<PathBuf>,
}

pub enum ParseOutcome {
    Run(Config),
    Help,
}

pub fn parse_args() -> Result<ParseOutcome, String> {
    let mut args = env::args().skip(1);
    let mut config = Config::default();

    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--help" | "-h" => return Ok(ParseOutcome::Help),
            "--root" => {
                let value = args.next().ok_or("--root requires a path")?;
                config.root = Some(PathBuf::from(value));
            }
            "--waivm" => {
                let value = args.next().ok_or("--waivm requires a path")?;
                config.waivm = Some(PathBuf::from(value));
            }
            "--file" => {
                let value = args.next().ok_or("--file requires a path")?;
                config.file = Some(PathBuf::from(value));
            }
            unknown => return Err(format!("unknown option: {unknown}")),
        }
    }

    Ok(ParseOutcome::Run(config))
}

pub fn usage() -> &'static str {
    "waivm-tui [--root <repo-root>] [--waivm <path-to-waivm>] [--file <program.wai|program.waibc>]\n\n\
Keyboard:\n\
  Up/Down or k/j     select program\n\
  Left/Right or [/]  change action\n\
  Enter              execute selected action\n\
  r/t/v/d/i          run/trace/verify/disassemble/info\n\
  x                  leave TUI and open interactive debugger\n\
  c                  clear output\n\
  ?                  toggle help\n\
  q or Esc           quit\n"
}

fn detect_root(input: Option<PathBuf>) -> io::Result<PathBuf> {
    if let Some(root) = input {
        return fs::canonicalize(root);
    }

    let cwd = env::current_dir()?;
    if cwd.join("examples").is_dir() && cwd.join("CMakeLists.txt").is_file() {
        return Ok(cwd);
    }

    if let Some(parent) = cwd.parent() {
        if parent.join("examples").is_dir() && parent.join("CMakeLists.txt").is_file() {
            return Ok(parent.to_path_buf());
        }
    }

    Ok(cwd)
}

fn detect_waivm(root: &Path, input: Option<PathBuf>) -> PathBuf {
    if let Some(path) = input {
        return path;
    }

    if let Ok(value) = env::var("WAIVM_BIN") {
        return PathBuf::from(value);
    }

    let candidates = [
        root.join("build/waivm"),
        root.join("build/src/waivm"),
        root.join("cmake-build-debug/waivm"),
        root.join("cmake-build-release/waivm"),
    ];

    for candidate in candidates {
        if candidate.is_file() {
            return candidate;
        }
    }

    PathBuf::from("waivm")
}

fn collect_programs(root: &Path) -> io::Result<Vec<Program>> {
    let mut programs = Vec::new();
    let examples = root.join("examples");

    if examples.is_dir() {
        for entry in fs::read_dir(examples)? {
            let entry = entry?;
            let path = entry.path();
            if is_program_file(&path) {
                let name = path
                    .file_name()
                    .and_then(|value| value.to_str())
                    .unwrap_or("program")
                    .to_string();
                programs.push(Program { name, path });
            }
        }
    }

    for entry in fs::read_dir(root)? {
        let entry = entry?;
        let path = entry.path();
        if is_program_file(&path) {
            let name = path
                .file_name()
                .and_then(|value| value.to_str())
                .unwrap_or("program")
                .to_string();
            programs.push(Program { name, path });
        }
    }

    programs.sort_by(|left, right| left.name.cmp(&right.name));
    programs.dedup_by(|left, right| left.path.as_os_str() == right.path.as_os_str());
    Ok(programs)
}

fn is_program_file(path: &Path) -> bool {
    matches!(
        path.extension().and_then(|value| value.to_str()),
        Some("wai") | Some("waibc")
    )
}
