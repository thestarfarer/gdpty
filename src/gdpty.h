#ifndef PTY_H
#define PTY_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class PTY : public RefCounted {
    GDCLASS(PTY, RefCounted);

private:
    int master_fd = -1;
    pid_t child_pid = -1;
    int cols = 80;
    int rows = 24;

protected:
    static void _bind_methods();

public:
    PTY();
    ~PTY();

    Error spawn(const String &command, const PackedStringArray &args);
    String read_output();
    void write_input(const String &data);
    void resize(int p_cols, int p_rows);
    void close();

    bool is_open() const;
    int get_pid() const;
    int get_cols() const;
    int get_rows() const;
};

}

#endif // PTY_H
