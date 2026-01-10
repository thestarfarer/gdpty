#include "gdpty.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <pty.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>

namespace godot {

void PTY::_bind_methods() {
    ClassDB::bind_method(D_METHOD("spawn", "command", "args"), &PTY::spawn);
    ClassDB::bind_method(D_METHOD("read_output"), &PTY::read_output);
    ClassDB::bind_method(D_METHOD("write_input", "data"), &PTY::write_input);
    ClassDB::bind_method(D_METHOD("resize", "cols", "rows"), &PTY::resize);
    ClassDB::bind_method(D_METHOD("close"), &PTY::close);
    ClassDB::bind_method(D_METHOD("is_open"), &PTY::is_open);
    ClassDB::bind_method(D_METHOD("get_pid"), &PTY::get_pid);
    ClassDB::bind_method(D_METHOD("get_cols"), &PTY::get_cols);
    ClassDB::bind_method(D_METHOD("get_rows"), &PTY::get_rows);
}

PTY::PTY() {
}

PTY::~PTY() {
    close();
}

Error PTY::spawn(const String &command, const PackedStringArray &args) {
    if (master_fd >= 0) {
        return ERR_ALREADY_IN_USE;
    }

    struct winsize ws;
    ws.ws_col = cols;
    ws.ws_row = rows;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    child_pid = forkpty(&master_fd, nullptr, nullptr, &ws);

    if (child_pid < 0) {
        UtilityFunctions::printerr("PTY: forkpty failed: ", strerror(errno));
        return ERR_CANT_CREATE;
    }

    if (child_pid == 0) {
        // Child process
        // Set environment variables for proper terminal support
        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        setenv("PROMPT_EOL_MARK", "", 1);  // Disable zsh's partial-line marker

        // Build argv array
        int argc = args.size() + 2;
        char **argv = new char*[argc];

        CharString cmd_utf8 = command.utf8();
        argv[0] = strdup(cmd_utf8.get_data());

        for (int i = 0; i < args.size(); i++) {
            CharString arg_utf8 = args[i].utf8();
            argv[i + 1] = strdup(arg_utf8.get_data());
        }
        argv[argc - 1] = nullptr;

        execvp(argv[0], argv);

        // If exec fails
        _exit(127);
    }

    // Parent process
    // Set non-blocking mode
    int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    return OK;
}

String PTY::read_output() {
    if (master_fd < 0) {
        return String();
    }

    char buf[8192];
    ssize_t n = ::read(master_fd, buf, sizeof(buf) - 1);

    if (n > 0) {
        buf[n] = '\0';
        return String::utf8(buf, n);
    }

    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        // Error or EOF
        if (errno == EIO) {
            // Child likely exited
            close();
        }
    }

    return String();
}

void PTY::write_input(const String &data) {
    if (master_fd < 0) {
        return;
    }

    CharString utf8 = data.utf8();
    (void)::write(master_fd, utf8.get_data(), utf8.length());
}

void PTY::resize(int p_cols, int p_rows) {
    cols = p_cols;
    rows = p_rows;

    if (master_fd < 0) {
        return;
    }

    struct winsize ws;
    ws.ws_col = cols;
    ws.ws_row = rows;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    ioctl(master_fd, TIOCSWINSZ, &ws);
}

void PTY::close() {
    if (master_fd >= 0) {
        ::close(master_fd);
        master_fd = -1;
    }

    if (child_pid > 0) {
        // Send SIGHUP then wait
        kill(child_pid, SIGHUP);
        int status;
        waitpid(child_pid, &status, WNOHANG);
        child_pid = -1;
    }
}

bool PTY::is_open() const {
    return master_fd >= 0;
}

int PTY::get_pid() const {
    return child_pid;
}

int PTY::get_cols() const {
    return cols;
}

int PTY::get_rows() const {
    return rows;
}

}
