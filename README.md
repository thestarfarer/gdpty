# gdpty

Real terminal emulation for Godot via PTY. Because sometimes you need actual shells floating in your VR space, not just text that looks like terminals.

Linux-only GDExtension that gives you proper pseudo-terminals with full TTY capabilities—colors, cursor control, the works. Built for Godot 4.5, probably works on 4.2+ but that's untested.

## What You Get

A clean `PTY` class that handles the messy bits:

```gdscript
var pty = PTY.new()

# Spawn whatever lives in your shell
pty.spawn("/bin/zsh", [])
pty.spawn("/usr/bin/python3", ["-i"])
pty.spawn("/usr/bin/htop", [])  # Because every VR space needs system monitoring

# Send keystrokes like they're real
pty.write_input("ls -la\n")
pty.write_input("\x03")  # Ctrl+C works too

# Non-blocking reads - won't freeze your frame
var output = pty.read_output()  # Empty string if buffer's dry

# Resize on the fly
pty.resize(120, 40)  # Cols, rows - default is 80x24

# Clean shutdown
pty.close()

# State checks
pty.is_open()   # Still alive?
pty.get_pid()   # Who's running in there?
pty.get_cols()  # Current dimensions
pty.get_rows()
```

The `read_output()` is non-blocking by design—call it in `_process()` or wherever you need fresh terminal data. Empty string means nothing new, not a dead process.

## Building

You'll need [godot-cpp](https://github.com/godotengine/godot-cpp) matching your Godot version.

```bash
# godot-cpp setup (or set GODOT_CPP_PATH if it lives elsewhere)
git clone --branch 4.5 https://github.com/godotengine/godot-cpp
cd godot-cpp && scons && cd ..

# Build this extension
cd gdpty
scons
```

Drops the binary in `bin/libgdpty.linux.template_debug.x86_64.so`. Release builds work the same way with `scons target=template_release`.

## Installation

Drop the whole `gdpty/` folder into your project's `addons/` directory. The `.gdextension` file tells Godot where to find the binary.

```
res://
├── addons/
│   └── gdpty/
│       ├── bin/
│       │   └── libgdpty.linux.*.so
│       └── gdpty.gdextension
```

## Under the Hood

Uses `forkpty()` for proper terminal emulation—this isn't just piping stdout, it's a real PTY with all the terminal protocol handling. The child process thinks it's talking to a real terminal, because it basically is.

RefCounted inheritance means Godot handles the lifecycle. No manual memory management needed—when your PTY reference drops, everything cleans up properly.

## Platform Reality

Linux only. This wraps POSIX PTY functions that simply don't exist on Windows. Mac could theoretically work since it has `forkpty()`, but that's untested territory. For Windows terminal needs, you'd need a completely different approach (probably ConPTY).

## Use Cases

- Terminal emulators in game UIs
- Interactive coding environments 
- System monitoring displays in VR
- Actual shells floating in 3D space
- Debug consoles that aren't just print statements
- Anything where you need a real terminal, not a text box

## License

MIT. Go wild.
