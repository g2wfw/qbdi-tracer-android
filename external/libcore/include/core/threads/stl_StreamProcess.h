//
// Created by xb on 2024/8/23.
//

#ifndef STL_STREAMPROCESS_H
#define STL_STREAMPROCESS_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifndef _WIN32

#include <sys/wait.h>

#endif

#include "../text/stl_String.h"
#include "../text/stl_StringRef.h"
#include "../text/stl_StringArray.h"


namespace stl {
    struct STL_EXPORT ProcessConfig {
        /// Buffer size for reading stdout and stderr. Default is 131072 (128 kB).
        std::size_t buffer_size = 131072;
        /// Set to true to inherit file descriptors from parent process. Default is false.
        /// On Windows: has no effect unless read_stdout==nullptr, read_stderr==nullptr and open_stdin==false.
        bool inherit_file_descriptors = false;

        /// If set, invoked when process stdout is closed.
        /// This call goes after last call to read_stdout().
        std::function<void()> on_stdout_close = nullptr;
        /// If set, invoked when process stderr is closed.
        /// This call goes after last call to read_stderr().
        std::function<void()> on_stderr_close = nullptr;

        /// On Windows only: controls how the process is started, mimics STARTUPINFO's wShowWindow.
        /// See: https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/ns-processthreadsapi-startupinfoa
        /// and https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-showwindow
        enum class ShowWindow {
            hide = 0,
            show_normal = 1,
            show_minimized = 2,
            maximize = 3,
            show_maximized = 3,
            show_no_activate = 4,
            show = 5,
            minimize = 6,
            show_min_no_active = 7,
            show_na = 8,
            restore = 9,
            show_default = 10,
            force_minimize = 11
        };

        /// On Windows only: controls how the window is shown.
        ShowWindow show_window{ShowWindow::show_default};

        /// Set to true to break out of flatpak sandbox by prepending all commands with `/usr/bin/flatpak-spawn --host`
        /// which will execute the command line on the host system.
        /// Requires the flatpak `org.freedesktop.Flatpak` portal to be opened for the current sandbox.
        /// See https://docs.flatpak.org/en/latest/flatpak-command-reference.html#flatpak-spawn.
        bool flatpak_spawn_host = false;
    };


    class STL_EXPORT StreamProcess {
    public:
#ifdef _WIN32
        typedef unsigned long id_type; // Process id type
        typedef void *fd_type; // File descriptor type

#else
        typedef pid_t id_type;
        typedef int fd_type;
#endif
        typedef std::unordered_map<String, String> environment_type;

    private:
        class Data {
        public:
            Data() noexcept;

            id_type id;
#ifdef _WIN32
            void *handle{nullptr};
#endif
            int exit_status{-1};
        };

    public:
        /// Starts a process with the environment of the calling process.
        StreamProcess(const StringArray &arguments, const String &path = "",
                      std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
                      std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
                      bool open_stdin = false,
                      const ProcessConfig &config = {}) noexcept
                : closed(true), read_stdout(std::move(read_stdout)), read_stderr(std::move(read_stderr)),
                  open_stdin(open_stdin), config(config) {
            open(arguments, path);
            async_read();
        }


        /// Starts a process with the environment of the calling process.
        StreamProcess(const String &command, const String &path = "",
                      std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
                      std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
                      bool open_stdin = false,
                      const ProcessConfig &config = {}) noexcept
                : closed(true), read_stdout(std::move(read_stdout)), read_stderr(std::move(read_stderr)),
                  open_stdin(open_stdin), config(config) {
            open(command, path);
            async_read();
        }

        /// Starts a process with specified environment.
        StreamProcess(const StringArray &arguments,
                      const String &path,
                      const environment_type &environment,
                      std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
                      std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
                      bool open_stdin = false,
                      const ProcessConfig &config = {}) noexcept
                : closed(true), read_stdout(std::move(read_stdout)), read_stderr(std::move(read_stderr)),
                  open_stdin(open_stdin), config(config) {
            open(arguments, path, &environment);
            async_read();
        }

        /// Starts a process with specified environment.
        StreamProcess(const String &command,
                      const String &path,
                      const environment_type &environment,
                      std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
                      std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
                      bool open_stdin = false,
                      const ProcessConfig &config = {}) noexcept
                : closed(true), read_stdout(std::move(read_stdout)), read_stderr(std::move(read_stderr)),
                  open_stdin(open_stdin), config(config) {
            open(command, path, &environment);
            async_read();
        }

#ifndef _WIN32

        /// Starts a process with the environment of the calling process.
        /// Supported on Unix-like systems only.
        /// Since the command line is not known to the Process object itself,
        /// this overload does not support the flatpak_spawn_host configuration.
        StreamProcess(const std::function<void()> &function,
                      std::function<void(const char *bytes, size_t n)> read_stdout = nullptr,
                      std::function<void(const char *bytes, size_t n)> read_stderr = nullptr,
                      bool open_stdin = false,
                      const ProcessConfig &config = {}) : closed(true), read_stdout(std::move(read_stdout)),
                                                          read_stderr(std::move(read_stderr)),
                                                          open_stdin(open_stdin), config(config) {
            if (config.flatpak_spawn_host)
                throw std::invalid_argument("Cannot break out of a flatpak sandbox with this overload.");
            open(function);
            async_read();
        }

#endif

        ~StreamProcess() noexcept;

        /// Get the process id of the started process.
        id_type get_id() const noexcept;

        /// Wait until process is finished, and return exit status.
        int get_exit_status() noexcept;

        /// If process is finished, returns true and sets the exit status. Returns false otherwise.
        bool try_get_exit_status(int &exit_status) noexcept;

        /// Write to stdin.
        bool write(const char *bytes, size_t n);

        /// Write to stdin. Convenience function using write(const char *, size_t).
        bool write(const std::string &str);

        /// Close stdin. If the process takes parameters from stdin, use this to notify that all parameters have been sent.
        void close_stdin() noexcept;

        /// Kill the process. force=true is only supported on Unix-like systems.
        void kill(bool force = false) noexcept;

        /// Kill a given process id. Use kill(bool force) instead if possible. force=true is only supported on Unix-like systems.
        static void kill(id_type id, bool force = false) noexcept;

#ifndef _WIN32

        /// Send the signal signum to the process.
        void signal(int signum) noexcept;

#endif

    private:
        Data data;
        bool closed;
        std::mutex close_mutex;
        std::function<void(const char *bytes, size_t n)> read_stdout;
        std::function<void(const char *bytes, size_t n)> read_stderr;
#ifndef _WIN32
        std::thread stdout_stderr_thread;
#else
        std::thread stdout_thread, stderr_thread;
#endif
        bool open_stdin;
        std::mutex stdin_mutex;

        ProcessConfig config;

        std::unique_ptr<fd_type> stdout_fd, stderr_fd, stdin_fd;

        id_type open(const StringArray &arguments, const String &path,
                     const environment_type *environment = nullptr) noexcept;

        id_type open(const String &command, const String &path,
                     const environment_type *environment = nullptr) noexcept;

#ifndef _WIN32

        id_type open(const std::function<void()> &function) noexcept;

#endif

        void async_read() noexcept;

        void close_fds() noexcept;
    };
}
#endif //STL_STREAMPROCESS_H
