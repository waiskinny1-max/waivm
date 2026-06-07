use crate::app::Action;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};
use std::process::{Command, Stdio};

pub fn execute(waivm: &Path, root: &Path, action: Action, file: &Path) -> Vec<String> {
    let mut lines = Vec::new();
    let file_for_action = match prepare_file_for_action(waivm, root, action, file, &mut lines) {
        Ok(path) => path,
        Err(err) => {
            lines.push(format!("error: {err}"));
            return lines;
        }
    };

    let command_name = action.cli_command();
    lines.push(format!(
        "$ {} {} {}",
        display_command(waivm),
        command_name,
        file_for_action.display()
    ));
    lines.push(String::new());

    match Command::new(waivm)
        .arg(command_name)
        .arg(&file_for_action)
        .current_dir(root)
        .output()
    {
        Ok(output) => {
            let code = output.status.code().map_or_else(|| "signal".to_string(), |value| value.to_string());
            lines.push(format!("exit code: {code}"));
            append_stream(&mut lines, "stdout", &output.stdout);
            append_stream(&mut lines, "stderr", &output.stderr);
        }
        Err(err) => {
            lines.push(format!("error: failed to execute waivm: {err}"));
            lines.push("hint: build the VM first or pass --waivm ./build/waivm".to_string());
        }
    }

    lines
}

pub fn launch_debugger(waivm: &Path, root: &Path, file: &Path) -> io::Result<i32> {
    let status = Command::new(waivm)
        .arg("debug")
        .arg(file)
        .current_dir(root)
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit())
        .status()?;

    Ok(status.code().unwrap_or(1))
}

fn prepare_file_for_action(
    waivm: &Path,
    root: &Path,
    action: Action,
    file: &Path,
    lines: &mut Vec<String>,
) -> io::Result<PathBuf> {
    let extension = file.extension().and_then(|value| value.to_str()).unwrap_or("");

    if action != Action::Info || extension == "waibc" {
        return Ok(file.to_path_buf());
    }

    let cache_dir = root.join(".waivm-tui");
    fs::create_dir_all(&cache_dir)?;

    let stem = file
        .file_stem()
        .and_then(|value| value.to_str())
        .unwrap_or("program");
    let output = cache_dir.join(format!("{stem}.waibc"));

    lines.push(format!(
        "$ {} asm {} -o {}",
        display_command(waivm),
        file.display(),
        output.display()
    ));

    let asm_result = Command::new(waivm)
        .arg("asm")
        .arg(file)
        .arg("-o")
        .arg(&output)
        .current_dir(root)
        .output()?;

    if !asm_result.status.success() {
        lines.push(format!("assembler failed: {}", asm_result.status));
        append_stream(lines, "stdout", &asm_result.stdout);
        append_stream(lines, "stderr", &asm_result.stderr);
        return Err(io::Error::new(io::ErrorKind::Other, "cannot create temporary bytecode"));
    }

    lines.push(format!("created temporary bytecode: {}", output.display()));
    lines.push(String::new());
    Ok(output)
}

fn append_stream(lines: &mut Vec<String>, name: &str, data: &[u8]) {
    if data.is_empty() {
        lines.push(format!("{name}: <empty>"));
        return;
    }

    lines.push(format!("{name}:"));
    let text = String::from_utf8_lossy(data);
    for line in text.lines() {
        lines.push(format!("  {line}"));
    }
}

fn display_command(path: &Path) -> String {
    path.to_string_lossy().to_string()
}
