//
//
//

#include "dispatch_syscall.h"
#include <spdlog/fmt/fmt.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "../memory_manager.h"

DispatchSyscall *DispatchSyscall::get_instance() {
    static DispatchSyscall syscall;
    return &syscall;
}

bool DispatchSyscall::dispatch_args(inst_trace_info_t *trace_info) {
    trace_info->fun_call->call_module_name = "kernel_syscall";
    auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
    auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
    auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
    auto arg3 = get_arg_register_value(&trace_info->pre_status, 3);
    auto arg4 = get_arg_register_value(&trace_info->pre_status, 4);
    auto arg5 = get_arg_register_value(&trace_info->pre_status, 5);
#ifdef __arm__
    auto sys_value = trace_info->pre_status.gpr_state.r7;

    static const std::unordered_map<uint32_t, std::pair<std::string, std::function<void()>>> handlers = {
        {__NR_exit, {"exit", [&]() {

            trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fork
        {__NR_fork, {"fork", [&]() {
            //pid_t fork()
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_read
        {__NR_read, {"read", [&]() {
            //ssize_t read(int fd, void *buf, size_t count)
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_write
        {__NR_write, {"write", [&]() {
            //ssize_t write(int fd, const void *buf, size_t count)
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_open
        {__NR_open, {"open", [&]() {
            //int open(const char *pathname, int flags, mode_t mode);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_close
        {__NR_close, {"close", [&]() {
            //int close(int fd);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_creat
        {__NR_creat, {"creat", [&]() {
            //int creat(const char *pathname, mode_t mode);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_link
        {__NR_unlink, {"unlink", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_execve, {"execve", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("argv={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("envp={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_chdir, {"chdir", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_mknod, {"mknod", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("dev={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_chmod, {"chmod", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_lchown, {"lchown", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_lseek, {"lseek", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getpid, {"getpid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_mount, {"mount", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("source={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("target={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("filesystemtype={}", read_string_from_address(arg2)));
            trace_info->fun_call->args.push_back(fmt::format("mountflags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("data={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setuid, {"setuid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getuid, {"getuid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ptrace, {"ptrace", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("data={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_pause, {"pause", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_access, {"access", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_nice, {"nice", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("inc={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sync, {"sync", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_kill, {"kill", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rename, {"rename", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("oldpath={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("newpath={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_mkdir, {"mkdir", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rmdir, {"rmdir", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_dup, {"dup", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("oldfd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_pipe, {"pipe", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pipefd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_times, {"times", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_brk, {"brk", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setgid, {"setgid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getgid, {"getgid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_geteuid, {"geteuid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getegid, {"getegid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_acct, {"acct", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("filename={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_umount2, {"umount2", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("target={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ioctl, {"ioctl", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fcntl, {"fcntl", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setpgid, {"setpgid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pgid={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_umask, {"umask", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_chroot, {"chroot", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ustat, {"ustat", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("dev={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("ubuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_dup2, {"dup2", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("oldfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("newfd={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getppid, {"getppid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getpgrp, {"getpgrp", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setsid, {"setsid", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigaction, {"sigaction", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("act={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("oldact={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setreuid, {"setreuid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setregid, {"setregid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigsuspend, {"sigsuspend", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigpending, {"sigpending", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sethostname, {"sethostname", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setrlimit, {"setrlimit", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("rlim={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getrusage, {"getrusage", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("usage={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_gettimeofday, {"gettimeofday", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        {__NR_settimeofday, {"settimeofday", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getgroups, {"getgroups", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setgroups, {"setgroups", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_symlink, {"symlink", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("target={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("linkpath={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_readlink, {"readlink", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("bufsiz={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_uselib, {"uselib", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("library={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_swapon, {"swapon", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("swapflags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_reboot, {"reboot", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("magic={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("magic2={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_munmap, {"munmap", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_truncate, {"truncate", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ftruncate, {"ftruncate", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fchmod, {"fchmod", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fchown, {"fchown", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getpriority, {"getpriority", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setpriority, {"setpriority", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("prio={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_statfs, {"statfs", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fstatfs, {"fstatfs", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_syslog, {"syslog", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setitimer, {"setitimer", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getitimer, {"getitimer", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_stat, {"stat", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_lstat, {"lstat", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fstat, {"fstat", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_vhangup, {"vhangup", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_wait4, {"wait4", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("options={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("rusage={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_swapoff, {"swapoff", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sysinfo, {"sysinfo", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("info={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fsync, {"fsync", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigreturn, {"sigreturn", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_clone, {"clone", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("child_stack={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("parent_tid={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("child_tid={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("tls={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setdomainname, {"setdomainname", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_uname, {"uname", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_adjtimex, {"adjtimex", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_mprotect, {"mprotect", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigprocmask, {"sigprocmask", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("how={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("oldset={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_init_module, {"init_module", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("module_image={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("param_values={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_delete_module, {"delete_module", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_quotactl, {"quotactl", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("special={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("id={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getpgid, {"getpgid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fchdir, {"fchdir", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_bdflush, {"bdflush", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("func={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("data={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sysfs, {"sysfs", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("option={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("arg1={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg2={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_personality, {"personality", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("persona={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setfsuid, {"setfsuid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setfsgid, {"setfsgid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR__llseek, {"_llseek", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("offset_high={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("offset_low={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("result={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getdents, {"getdents", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("dirp={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR__newselect, {"_newselect", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("readfds={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("writefds={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("exceptfds={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_flock, {"flock", [&]() {
            // int flock(int fd, int cmd, struct flock *lock);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("lock={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_msync
        {__NR_msync, {"msync", [&]() {
            // int msync(void *addr, size_t length, int flags);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_readv
        {__NR_readv, {"readv", [&]() {
            // ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_writev
        {__NR_writev, {"writev", [&]() {
            // ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int getsid(pid_t pid);
        {__NR_getsid, {"getsid", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int fdatasync(int fd);
        {__NR_fdatasync, {"fdatasync", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int _sysctl(struct __sysctl_args *args);
        {__NR__sysctl, {"_sysctl", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("args={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int mlock(const void *addr, size_t len);
        {__NR_mlock, {"mlock", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int munlock(const void *addr, size_t len);
        {__NR_munlock, {"munlock", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int mlockall(int flags);
        {__NR_mlockall, {"mlockall", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int munlockall(void);
        {__NR_munlockall, {"munlockall", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_setparam
        {__NR_sched_setparam, {"sched_setparam", [&]() {
            // int sched_setparam(pid_t pid, const struct sched_param *param);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_getparam
        {__NR_sched_getparam, {"sched_getparam", [&]() {
            // int sched_getparam(pid_t pid, struct sched_param *param);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_setscheduler
        {__NR_sched_setscheduler, {"sched_setscheduler", [&]() {
            // int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg2));
        }}},
        //__NR_sched_getscheduler
        {__NR_sched_getscheduler, {"sched_getscheduler", [&]() {
            // int sched_getscheduler(pid_t pid);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_yield
        {__NR_sched_yield, {"sched_yield", [&]() {
            // int sched_yield(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_get_priority_max
        {__NR_sched_get_priority_max, {"sched_get_priority_max", [&]() {
            // int sched_get_priority_max(int policy);
            trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
        }}},
        //__NR_sched_get_priority_min
        {__NR_sched_get_priority_min, {"sched_get_priority_min", [&]() {
            // int sched_get_priority_min(int policy);
            trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_rr_get_interval
        {__NR_sched_rr_get_interval, {"sched_rr_get_interval", [&]() {
            // int sched_rr_get_interval(pid_t pid, struct timespec *ts);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("ts={:#x}", arg1));
        }}},
        //__NR_nanosleep
        {__NR_nanosleep, {"nanosleep", [&]() {
            // int nanosleep(const struct timespec *req, struct timespec *rem);
            trace_info->fun_call->args.push_back(fmt::format("req={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("rem={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_mremap
        {__NR_mremap, {"mremap", [&]() {
            // void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, void *new_address);
            trace_info->fun_call->args.push_back(fmt::format("old_address={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("old_size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("new_size={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("new_address={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_setresuid
        {__NR_setresuid, {"setresuid", [&]() {
            // int setresuid(uid_t ruid, uid_t euid, uid_t suid);
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getresuid
        {__NR_getresuid, {"getresuid", [&]() {
            // int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_poll
        {__NR_poll, {"poll", [&]() {
            // int poll(struct pollfd *fds, nfds_t nfds, int timeout);
            trace_info->fun_call->args.push_back(fmt::format("fds={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_nfsservctl
        {__NR_nfsservctl, {"nfsservctl", [&]() {
            // int nfsservctl(int cmd, void *arg, void *buf, size_t buflen);
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("buflen={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_setresgid
        {__NR_setresgid, {"setresgid", [&]() {
            // int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
            trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sgid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_prctl, {"prctl", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("option={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("arg2={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg3={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("arg4={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("arg5={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int rt_sigreturn(void);
        {__NR_rt_sigreturn, {"rt_sigreturn", [&]() {
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
        {__NR_rt_sigaction, {"rt_sigaction", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("signum={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("act={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("oldact={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
        {__NR_rt_sigprocmask, {"rt_sigprocmask", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("how={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("oldset={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rt_sigpending, {"rt_sigpending", [&]() {
            // int rt_sigpending(sigset_t *set, size_t sigsetsize);
            trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rt_sigtimedwait, {"rt_sigtimedwait", [&]() {
            // int rt_sigtimedwait(const sigset_t *uthese, siginfo_t *uinfo, const struct timespec *uts, size_t sigsetsize);
            trace_info->fun_call->args.push_back(fmt::format("uthese={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("uinfo={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("uts={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rt_sigqueueinfo, {"rt_sigqueueinfo", [&]() {
            // int rt_sigqueueinfo(pid_t pid, int sig, siginfo_t *uinfo);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("uinfo={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_rt_sigsuspend, {"rt_sigsuspend", [&]() {
            // int rt_sigsuspend(sigset_t *unewset, size_t sigsetsize);
            trace_info->fun_call->args.push_back(fmt::format("unewset={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_pread64, {"pread64", [&]() {
            // ssize_t pread64(int fd, void *buf, size_t count, off_t offset);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_pwrite64, {"pwrite64", [&]() {
            // ssize_t pwrite64(int fd, const void *buf, size_t count, off_t offset);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_chown, {"chown", [&]() {
            // int chown(const char *pathname, uid_t owner, gid_t group);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getcwd, {"getcwd", [&]() {
            // char *getcwd(char *buf, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->ret_type = kString;
        }}},
        {__NR_capget, {"capget", [&]() {
            // int capget(cap_user_header_t hdrp, cap_user_data_t datap);
            trace_info->fun_call->args.push_back(fmt::format("hdrp={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("datap={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_capset, {"capset", [&]() {
            // int capset(cap_user_header_t hdrp, const cap_user_data_t datap);
            trace_info->fun_call->args.push_back(fmt::format("hdrp={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("datap={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sigaltstack, {"sigaltstack", [&]() {
            // int sigaltstack(const stack_t *ss, stack_t *oss);
            trace_info->fun_call->args.push_back(fmt::format("ss={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("oss={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_sendfile, {"sendfile", [&]() {
            // ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
            trace_info->fun_call->args.push_back(fmt::format("out_fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("in_fd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_vfork, {"vfork", [&]() {
            // pid_t vfork(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ugetrlimit, {"ugetrlimit", [&]() {
            // int ugetrlimit(int resource, struct rlimit *rlim);
            trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("rlim={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_mmap2, {"mmap2", [&]() {
            // void *mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("pgoffset={:#x}", arg5));
            trace_info->fun_call->ret_type = kPointer;
        }}},
        {__NR_truncate64, {"truncate64", [&]() {
            // int truncate64(const char *path, off_t length);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ftruncate64, {"ftruncate64", [&]() {
            // int ftruncate64(int fd, off_t length);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_stat64, {"stat64", [&]() {
            // int stat64(const char *pathname, struct stat64 *statbuf);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_lstat64, {"lstat64", [&]() {
            // int lstat64(const char *pathname, struct stat64 *statbuf);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fstat64, {"fstat64", [&]() {
            // int fstat64(int fd, struct stat64 *statbuf);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_lchown32, {"lchown32", [&]() {
            // int lchown32(const char *pathname, uid_t owner, gid_t group);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getuid32, {"getuid32", [&]() {
            // uid_t getuid32(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getgid32, {"getgid32", [&]() {
            // gid_t getgid32(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_geteuid32, {"geteuid32", [&]() {
            // uid_t geteuid32(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getegid32, {"getegid32", [&]() {
            // gid_t getegid32(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setreuid32, {"setreuid32", [&]() {
            // int setreuid32(uid_t ruid, uid_t euid);
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setregid32, {"setregid32", [&]() {
            // int setregid32(gid_t rgid, gid_t egid);
            trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getgroups32, {"getgroups32", [&]() {
            // int getgroups32(int size, gid_t list[]);
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setgroups32, {"setgroups32", [&]() {
            // int setgroups32(int size, const gid_t *list);
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_fchown32, {"fchown32", [&]() {
            // int fchown32(int fd, uid_t owner, gid_t group);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setresuid32, {"setresuid32", [&]() {
            // int setresuid32(uid_t ruid, uid_t euid, uid_t suid);
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getresuid32, {"getresuid32", [&]() {
            // int getresuid32(uid_t *ruid, uid_t *euid, uid_t *suid);
            trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setresgid32, {"setresgid32", [&]() {
            // int setresgid32(gid_t rgid, gid_t egid, gid_t sgid);
            trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sgid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_getresgid32, {"getresgid32", [&]() {
            // int getresgid32(gid_t *rgid, gid_t *egid, gid_t *sgid);
            trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sgid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_chown32, {"chown32", [&]() {
            // int chown32(const char *pathname, uid_t owner, gid_t group);
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("owner={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("group={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setuid32, {"setuid32", [&]() {
            // int setuid32(uid_t uid);
            trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setgid32, {"setgid32", [&]() {
            // int setgid32(gid_t gid);
            trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_setfsuid32, {"setfsuid32", [&]() {
            // int setfsuid32(uid_t fsuid);
            trace_info->fun_call->args.push_back(fmt::format("fsuid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_setfsgid32 (216)
        {__NR_setfsgid32, {"setfsgid32", [&]() {
            // int setfsgid32(gid_t fsgid);
            trace_info->fun_call->args.push_back(fmt::format("fsgid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getdents64 (217)
        {__NR_getdents64, {"getdents64", [&]() {
            // int getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("dirp={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_pivot_root (218)
        {__NR_pivot_root, {"pivot_root", [&]() {
            // int pivot_root(const char __user *new_root, const char __user *put_old);
            trace_info->fun_call->args.push_back(fmt::format("new_root={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("put_old={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_mincore (219)
        {__NR_mincore, {"mincore", [&]() {
            // int mincore(unsigned long start, size_t len, unsigned char __user *vec);
            trace_info->fun_call->args.push_back(fmt::format("start={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("vec={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_madvise (220)

        {__NR_madvise, {"madvise", [&]() {
            // int madvise(unsigned long addr, size_t len, int advice);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("advice={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_fcntl64 (221)
        {__NR_fcntl64, {"fcntl64", [&]() {
            // int fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_gettid (224)
        {__NR_gettid, {"gettid", [&]() {
            // pid_t gettid(void);
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_readahead (225)

        {__NR_readahead, {"readahead", [&]() {
            // ssize_t readahead(int fd, loff_)
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_setxattr (226)
        {__NR_setxattr, {"setxattr", [&]() {
            // int setxattr(const char __user *path, const char __user *name, const void __user *value, size_t size, int flags);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_lsetxattr (227)

        {__NR_setxattr, {"setxattr", [&]() {
            // int lsetxattr(const char __user *path, const char __user *name, const void __user *value, size_t size, int flags)
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_fsetxattr (228)

        {__NR_setxattr, {"setxattr", [&]() {
            // int fsetxattr(int fd, const char __user *name, const void __user *value, size_t size, int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_getxattr (229)

        {__NR_getxattr, {"getxattr", [&]() {
            // ssize_t getxattr(const char __user *path, const char __user *name, void __user *value, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;

        }}},
        {__NR_lgetxattr, {"lgetxattr", [&]() {
            // ssize_t lgetxattr(const char __user *path, const char __user *name, void __user *value, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        {__NR_fgetxattr, {"fgetxattr", [&]() {
            // ssize_t fgetxattr(int fd, const char __user *name, void __user *value, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        {__NR_listxattr, {"listxattr", [&]() {
            // ssize_t listxattr(const char __user *path, char __user *list, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_llistxattr (233)
        {__NR_llistxattr, {"llistxattr", [&]() {
            // ssize_t llistxattr(const char __user *path, char __user *list, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_flistxattr (234)
        {__NR_flistxattr, {"flistxattr", [&]() {
            // ssize_t flistxattr(int fd, char __user *list, size_t size);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_removexattr (235)
        {__NR_removexattr, {"removexattr", [&]() {
            // int removexattr(const char __user *path, const char __user *name);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_lremovexattr (236)
        {__NR_lremovexattr, {"lremovexattr", [&]() {
            // int lremovexattr(const char __user *path, const char __user *name);
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fremovexattr (237)
        {__NR_fremovexattr, {"fremovexattr", [&]() {
            // int fremovexattr(int fd, const char __user *name);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg1)));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_tkill (238)
        {__NR_tkill, {"tkill", [&]() {
            trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_sendfile64 (239)
        {__NR_sendfile64, {"sendfile64", [&]() {
            // ssize_t sendfile64(int out_fd, int in_fd, off64_t __user *offset, size_t count);
            trace_info->fun_call->args.push_back(fmt::format("out_fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("in_fd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_futex (240)
        {__NR_futex, {"futex", [&]() {
            // int futex(u32 __user *uaddr, int op, u32 val, struct timespec __user *utime, u32 __user *uaddr2, u32 val3);
            trace_info->fun_call->args.push_back(fmt::format("uaddr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("op={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("val={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("utime={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("uaddr2={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("val3={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_setaffinity (241)
        {__NR_sched_setaffinity, {"sched_setaffinity", [&]() {
            // int sched_setaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sched_getaffinity (242)
        {__NR_sched_getaffinity, {"sched_getaffinity", [&]() {
            // int sched_getaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_io_setup (243)
        {__NR_io_setup, {"io_setup", [&]() {
            // int io_setup(unsigned nr_events, aio_context_t __user *ctx);
            trace_info->fun_call->args.push_back(fmt::format("nr_events={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("ctx={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_io_destroy (244)
        {__NR_io_destroy, {"io_destroy", [&]() {
            // int io_destroy(aio_context_t ctx);
            trace_info->fun_call->args.push_back(fmt::format("ctx={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_io_getevents (245)
        {__NR_io_getevents, {"io_getevents", [&]() {
            // int io_getevents(aio_context_t ctx_id, long min_nr, long nr, struct io_event __user *events, struct timespec __user *timeout);
            trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("min_nr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("nr={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("events={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_io_submit (246)
        {__NR_io_submit, {"io_submit", [&]() {
            // int io_submit(aio_context_t ctx_id, long nr, struct iocb __user *iocbpp);
            trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iocbpp={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_io_cancel (247)
        {__NR_io_cancel, {"io_cancel", [&]() {
            // int io_cancel(aio_context_t ctx_id, struct iocb __user *iocb, struct io_cancel_event __user *result);
            trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iocb={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("result={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_exit_group (248)
        {__NR_exit_group, {"exit_group", [&]() {
            // void exit_group(int error_code);
            trace_info->fun_call->args.push_back(fmt::format("error_code={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_lookup_dcookie (249)
        {__NR_lookup_dcookie, {"lookup_dcookie", [&]() {
            // long lookup_dcookie(u64 cookie64, char __user *buf, size_t len);
            trace_info->fun_call->args.push_back(fmt::format("cookie64={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_epoll_create (250)
        {__NR_epoll_create, {"epoll_create", [&]() {
            // int epoll_create(int size);
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_epoll_ctl (251)
        {__NR_epoll_ctl, {"epoll_ctl", [&]() {
            // int epoll_ctl(int epfd, int op, int fd, struct epoll_event __user *event);
            trace_info->fun_call->args.push_back(fmt::format("epfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("op={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("event={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_epoll_wait (252)
        {__NR_epoll_wait, {"epoll_wait", [&]() {
            // long epoll_wait(int epfd, struct epoll_event __user *events, int maxevents, int timeout);
            trace_info->fun_call->args.push_back(fmt::format("epfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("events={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("maxevents={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_remap_file_pages (253)
        {__NR_remap_file_pages, {"remap_file_pages", [&]() {
            // long remap_file_pages(unsigned long start, unsigned long size, unsigned long prot, unsigned long pgoff, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("start={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("pgoff={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_set_tid_address (256)
        {__NR_set_tid_address, {"set_tid_address", [&]() {
            // int set_tid_address(int __user *tidptr);
            trace_info->fun_call->args.push_back(fmt::format("tidptr={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timer_create (257)
        {__NR_timer_create, {"timer_create", [&]() {
            // int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sevp={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timer_gettime
        {__NR_timer_gettime, {"timer_gettime", [&]() {
            // int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
            trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timer_getoverrun
        {__NR_timer_getoverrun, {"timer_getoverrun", [&]() {
            // int timer_getoverrun(timer_t timerid);
            trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timer_settime
        {__NR_timer_settime, {"timer_settime", [&]() {
            // int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
            trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timer_delete
        {__NR_timer_delete, {"timer_delete", [&]() {
            // int timer_delete(timer_t timerid);
            trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_clock_settime
        {__NR_clock_settime, {"clock_settime", [&]() {
            // int clock_settime(clockid_t clockid, const struct timespec *tp);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tp={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_clock_gettime
        {__NR_clock_gettime, {"clock_gettime", [&]() {
            // int clock_gettime(clockid_t clockid, struct timespec *tp);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tp={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_clock_getres
        {__NR_clock_getres, {"clock_getres", [&]() {
            // int clock_getres(clockid_t clockid, struct timespec *res);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("res={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_clock_nanosleep
        {__NR_clock_nanosleep, {"clock_nanosleep", [&]() {
            // int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("remain={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_statfs64 (266)
        {__NR_statfs64, {"statfs64", [&]() {
            // int statfs64(const char *path, struct statfs64 *buf);
            trace_info->fun_call->args.push_back(fmt::format("path={:#x}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_fstatfs64 (267)
        {__NR_fstatfs64, {"fstatfs64", [&]() {
            // int fstatfs64(int fd, struct statfs64 *buf);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_tgkill (268)
        {__NR_tgkill, {"tgkill", [&]() {
            // int tgkill(int tgid, int tid, int sig);
            trace_info->fun_call->args.push_back(fmt::format("tgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_utimes (269)
        {__NR_utimes, {"utimes", [&]() {
            // int utimes(const char *filename, const struct timeval *tv);
            trace_info->fun_call->args.push_back(fmt::format("filename={:#x}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_arm_fadvise64_64 (270)
        {__NR_arm_fadvise64_64, {"arm_fadvise64_64", [&]() {
            // int arm_fadvise64_64(int fd, int advice, off64_t offset, off64_t len);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("advice={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_mq_open
        {__NR_mq_open, {"mq_open", [&]() {
            // mqd_t mq_open(const char *_Nonnull name, int oflag, ...);
            trace_info->fun_call->args.push_back(fmt::format("name={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("oflag={}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_mq_unlink
        {__NR_mq_unlink, {"mq_unlink", [&]() {
            // int mq_unlink(const char *_Nonnull name);
            trace_info->fun_call->args.push_back(fmt::format("name={:#x}", read_string_from_address(arg0)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_mq_timedsend 182
        {__NR_mq_timedsend, {"mq_timedsend", [&]() {
            // ssize_t mq_timedsend(mqd_t mqdes, const char *_Nonnull msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *_Nonnull abs_timeout);
            trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msg_ptr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("msg_len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("msg_prio={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("abs_timeout={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_mq_timedreceive 183
        {__NR_mq_timedreceive, {"mq_timedreceive", [&]() {
            // ssize_t mq_timedreceive(mqd_t mqdes, char *_Nonnull msg_ptr, size_t msg_len, unsigned int *_Nullable msg_prio, const struct timespec *_Nonnull abs_timeout);
            trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msg_ptr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("msg_len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("msg_prio={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("abs_timeout={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_mq_notify 184
        {__NR_mq_notify, {"mq_notify", [&]() {
            // int mq_notify(mqd_t mqdes, const struct sigevent *_Nullable notification);
            trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("notification={:#x}", arg1));
        }}},
        // __NR_mq_getsetattr 185
        {__NR_mq_getsetattr, {"mq_getsetattr", [&]() {
            // int mq_getsetattr(mqd_t mqdes, const struct mq_attr *_Nonnull newattr, struct mq_attr *_Nonnull oldattr);
            trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("newattr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("oldattr={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_waitid (280)
        {__NR_waitid, {"waitid", [&]() {
            // int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
            trace_info->fun_call->args.push_back(fmt::format("idtype={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("id={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("infop={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("options={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_socket (281)
        {__NR_socket, {"socket", [&]() {
            // int socket(int domain, int type, int protocol);
            trace_info->fun_call->args.push_back(fmt::format("domain={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("protocol={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_bind (282)
        {__NR_bind, {"bind", [&]() {
            // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_connect (283)
        {__NR_connect, {"connect", [&]() {
            // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_listen (284)
        {__NR_listen, {"listen", [&]() {
            // int listen(int sockfd, int backlog);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("backlog={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_accept (285)
        {__NR_accept, {"accept", [&]() {
            // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getsockname (286)
        {__NR_getsockname, {"getsockname", [&]() {
            // int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getpeername (287)

        {__NR_getpeername, {"getpeername", [&]() {
            // int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_socketpair (288)
        {__NR_socketpair, {"socketpair", [&]() {
            // int socketpair(int domain, int type, int protocol, int sv[2]);
            trace_info->fun_call->args.push_back(fmt::format("domain={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("protocol={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("sv={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_send (289)
        {__NR_send, {"send", [&]() {
            // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sendto (290)
        {__NR_sendto, {"sendto", [&]() {
            // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("dest_addr={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_recv (291)
        {__NR_recv, {"recv", [&]() {
            // ssize_t recv(int sockfd, void *buf)
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_recvfrom (292)

        {__NR_recvfrom, {"recvfrom", [&]() {
            // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("src_addr={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_shutdown (293)
        {__NR_shutdown, {"shutdown", [&]() {
            // int shutdown(int sockfd, int how);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("how={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_setsockopt (294)
        {__NR_setsockopt, {"setsockopt", [&]() {
            // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("level={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("optname={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("optval={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("optlen={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getsockopt (295)
        {__NR_getsockopt, {"getsockopt", [&]() {
            // int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("level={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("optname={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("optval={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("optlen={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_sendmsg (296)
        {__NR_sendmsg, {"sendmsg", [&]() {
            // ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msg={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_recvmsg (297)
        {__NR_recvmsg, {"recvmsg", [&]() {
            // ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msg={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        {__NR_semop, {"semop", [&]() {
            // int semop(int semid, struct sembuf *sops, size_t nsops);
            trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sops={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("nsops={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_shmget 194
        {__NR_shmget, {"shmget", [&]() {
            // int shmget(key_t key, size_t size, int shmflg);
            trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("shmflg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_shmctl 195
        {__NR_shmctl, {"shmctl", [&]() {
            // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
            trace_info->fun_call->args.push_back(fmt::format("shmid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_msgsnd, {"msgsnd", [&]() {
            // int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
            trace_info->fun_call->args.push_back(fmt::format("msqid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msgp={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("msgsz={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("msgflg={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        {__NR_semget, {"semget", [&]() {
            // int semget(key_t key, int nsems, int semflg);
            trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nsems={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("semflg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_semctl, {"semctl", [&]() {
            // int semctl(int semid, int semnum, int cmd, ...);
            trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("semnum={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg2));
            if (arg3 != 0) {
                trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg3));
            }
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_shmat (305)
        //__NR_shmdt (306)
        //__NR_shmget (307)
        //__NR_shmctl (308)
        // __NR_shmget 194
        {__NR_shmget, {"shmget", [&]() {
            // int shmget(key_t key, size_t size, int shmflg);
            trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("shmflg={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_shmctl 195
        {__NR_shmctl, {"shmctl", [&]() {
            // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
            trace_info->fun_call->args.push_back(fmt::format("shmid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_shmat 196
        {__NR_shmat, {"shmat", [&]() {
            // void *shmat(int shmid, const void *shmaddr, int shmflg);
            trace_info->fun_call->args.push_back(fmt::format("shmid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("shmaddr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("shmflg={:#x}", arg2));
            trace_info->fun_call->ret_type = kPointer;
        }}},
        // __NR_shmdt 197
        {__NR_shmdt, {"shmdt", [&]() {
            // int shmdt(const void *shmaddr);
            trace_info->fun_call->args.push_back(fmt::format("shmaddr={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_add_key (309)
        //__NR_request_key (310)
        //__NR_keyctl (311)
        {__NR_add_key, {"add_key", [&]() {
            // int add_key(const char *type, const char *description, const void *payload, size_t plen, key_serial_t destringid);
            trace_info->fun_call->args.push_back(fmt::format("type={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("description={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("payload={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("plen={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("destringid={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;

        }}},
        // __NR_request_key 218
        {__NR_request_key, {"request_key", [&]() {
            // key_serial_t request_key(const char *type, const char *description, const char *callout_info, key_serial_t destringid);
            trace_info->fun_call->args.push_back(fmt::format("type={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("description={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("callout_info={}", read_string_from_address(arg2)));
            trace_info->fun_call->args.push_back(fmt::format("destringid={}", read_string_from_address(arg3)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_keyctl 219
        {__NR_keyctl, {"keyctl", [&]() {
            // long keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("arg2={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("arg3={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("arg4={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("arg5={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_semtimedop (312)
        {__NR_semtimedop, {"semtimedop", [&]() {
            // int semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout);
            trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("sops={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("nsops={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_vserver (313)
        {__NR_vserver, {"vserver", [&]() {
            // long vserver(unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3, unsigned long a4, unsigned long a5);
            trace_info->fun_call->args.push_back(fmt::format("a0={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("a1={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("a2={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("a3={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("a4={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("a5={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_ioprio_set (314)
        //__NR_ioprio_get (315)
        {
            __NR_ioprio_set, {"ioprio_set", [&]() {
                //int ioprio_set(int __which, int __who, int __ioprio);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("ioprio={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}
        },
        {
            __NR_ioprio_get, {"ioprio_get", [&]() {
                //int ioprio_get(int __which, int __who);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}
        },
        //__NR_inotify_init (316)
        {
            __NR_inotify_init, {"inotify_init", [&]() {
                //int inotify_init(void);
                trace_info->fun_call->ret_type = kNumber;
            }}
        },
        //__NR_inotify_add_watch (317)
        //__NR_inotify_rm_watch (318)
        {
            __NR_inotify_add_watch, {"inotify_add_watch", [&]() {
                //int inotify_add_watch(int __fd, const char* __path, uint32_t __mask);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}
        },
        {
            __NR_inotify_rm_watch, {"inotify_rm_watch", [&]() {
                //int inotify_rm_watch(int __fd, int __wd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("wd={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}
        },

        //__NR_mbind (319)
        //__NR_get_mempolicy (320)
        //__NR_set_mempolicy (321)
        // __NR_mbind 235
        {__NR_mbind, {"mbind", [&]() {
            // int mbind(void *addr, size_t len, int mode, const unsigned long *nodemask, unsigned long maxnode, unsigned flags);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("nodemask={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_get_mempolicy 236
        {__NR_get_mempolicy, {"get_mempolicy", [&]() {
            // int get_mempolicy(int *policy, const unsigned long *nmask, unsigned long maxnode, void *addr, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nmask={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_set_mempolicy 237
        {__NR_set_mempolicy, {"set_mempolicy", [&]() {
            // int set_mempolicy(int mode, const unsigned long *nmask, unsigned long maxnode);
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nmask={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_openat (322)
        {__NR_openat, {"openat", [&]() {
            //int openat(int __dirfd, const char* __path, int __flags, mode_t __mode);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_mkdirat (323)
        {__NR_mkdirat, {"mkdirat", [&]() {
            //int mkdirat(int __fd, const char* __path, mode_t __mode);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_mknodat (324)
        {__NR_mknodat, {"mknodat", [&]() {
            //int mknodat(int __fd, const char* __path, mode_t __mode, dev_t __dev);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("dev={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fchownat (325)
        {__NR_fchownat, {"fchownat", [&]() {
            //int fchownat(int __dirfd, const char* __path, uid_t __uid, gid_t __gid, int __flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_futimesat (326)
        {__NR_futimesat, {"futimesat", [&]() {
            //int futimesat(int __dirfd, const char* __path, const struct timeval __tvp[2]);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("tvp={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fstatat64 (327)
        {__NR_fstatat64, {"fstatat64", [&]() {
            //int fstatat64(int __dirfd, const char* __path, struct stat64* __stbuf, int __flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("stbuf={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_unlinkat (328)
        {__NR_unlinkat, {"unlinkat", [&]() {
            //int unlinkat(int __fd, const char* __path, int __flag);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("flag={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_renameat (329)
        {__NR_renameat, {"renameat", [&]() {
            //int renameat(int __oldfd, const char* __oldpath, int __newfd, const char* __newpath);
            trace_info->fun_call->args.push_back(fmt::format("oldfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("oldpath={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("newfd={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("newpath={}", read_string_from_address(arg3)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_linkat (330)
        //__NR_symlinkat (331)
        {__NR_symlinkat, {"symlinkat", [&]() {
            //int symlinkat(const char* __oldpath, int __fd, const char* __newpath);
            trace_info->fun_call->args.push_back(fmt::format("oldpath={}", read_string_from_address(arg0)));
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("newpath={}", read_string_from_address(arg2)));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_linkat, {"linkat", [&]() {
            //int linkat(int __fromfd, const char* __frompath, int __tofd,)
            trace_info->fun_call->args.push_back(fmt::format("fromfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("frompath={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("tofd={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("topath={}", read_string_from_address(arg3)));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_readlinkat (332)
        {__NR_readlinkat, {"readlinkat", [&]() {
            // ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("bufsiz={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fchmodat (333)
        {__NR_fchmodat, {"fchmodat", [&]() {
            //int fchmodat(int __dirfd, const char* __path, mode_t __mode, int __flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_faccessat (334)
        {__NR_faccessat, {"faccessat", [&]() {
            //int faccessat(int __dirfd, const char* __path, int __mode, int __flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("path={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_pselect6 (335)
        //__NR_ppoll (336)
        {__NR_pselect6, {"pselect6", [&]() {
            // int pselect6(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec *timeout, const sigset_t *sigmask);
            trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("readfds={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("writefds={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("exceptfds={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("sigmask={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_ppoll, {"ppoll", [&]() {
            // int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask);
            trace_info->fun_call->args.push_back(fmt::format("fds={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("sigmask={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_unshare (337)
        {__NR_unshare, {"unshare", [&]() {
            // int unshare(unsigned long unshare_flags);
            trace_info->fun_call->args.push_back(fmt::format("unshare_flags={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_set_robust_list (338)
        //__NR_get_robust_list (339)
        {__NR_set_robust_list, {"set_robust_list", [&]() {
            // long set_robust_list(struct robust_list_head *head, size_t len);
            trace_info->fun_call->args.push_back(fmt::format("head={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_get_robust_list, {"get_robust_list", [&]() {
            // long get_robust_list(int pid, struct robust_list_head **head_ptr, size_t *len_ptr);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("head_ptr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len_ptr={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_splice (340)
        //__NR_arm_sync_file_range (341)
        //__NR_tee (342)
        {__NR_splice, {"splice", [&]() {
            // ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("off_in={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("off_out={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_tee, {"tee", [&]() {
            // ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_vmsplice (343)
        {__NR_vmsplice, {"vmsplice", [&]() {
            // ssize_t vmsplice(int fd, const struct iovec *iov, unsigned long nr_segs, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("nr_segs={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_move_pages (344)
        {__NR_move_pages, {"move_pages", [&]() {
            // int move_pages(pid_t pid, int nr_pages, const void *pages[], const int *nodes[], int *status[], int flags);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nr_pages={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("pages={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("nodes={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_getcpu (345)
        {__NR_getcpu, {"getcpu", [&]() {
            //int getcpu(unsigned int *_Nullable cpu, unsigned int *_Nullable node);
            trace_info->fun_call->args.push_back(fmt::format("cpu={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("node={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_epoll_pwait (346)
        {__NR_epoll_pwait, {"epoll_pwait", [&]() {
            //int epoll_pwait(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout, const sigset_t* __ss);
            trace_info->fun_call->args.push_back(fmt::format("epfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("events={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("maxevents={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("ss={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_kexec_load (347)
        {__NR_kexec_load, {"kexec_load", [&]() {
            // long kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment *segments, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("entry={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nr_segments={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("segments={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_utimensat (348)
        {__NR_utimensat, {"utimensat", [&]() {
            // int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pathname={}", read_string_from_address(arg1)));
            trace_info->fun_call->args.push_back(fmt::format("times={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_signalfd (349)
        {__NR_signalfd, {"signalfd", [&]() {
            // int signalfd(int fd, const struct signalfd_siginfo *mask, size_t sizemask);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sizemask={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timerfd_create (350)
        {__NR_timerfd_create, {"timerfd_create", [&]() {
            // int timerfd_create(int clockid, int flags);
            trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_eventfd (351)
        {__NR_eventfd, {"eventfd", [&]() {
            // int eventfd(unsigned int count, int flags);
            trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fallocate (352)
        {__NR_fallocate, {"fallocate", [&]() {
            // int fallocate(int fd, int mode, off_t offset, off_t len);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_timerfd_settime (353)

        //__NR_timerfd_gettime (354)
        {__NR_timerfd_settime, {"timerfd_settime", [&]() {
            // int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_timerfd_gettime, {"timerfd_gettime", [&]() {
            // int timerfd_gettime(int fd, struct itimerspec *curr_value);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_signalfd4 (355)
        {__NR_signalfd4, {"signalfd4", [&]() {
            // int signalfd4(int fd, const sigset_t *mask, size_t sizemask, int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sizemask={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_eventfd2 (356)
        //__NR_epoll_create1 (357)
        {__NR_eventfd2, {"eventfd2", [&]() {
            //int eventfd(unsigned int initval, int flags);
            trace_info->fun_call->args.push_back(fmt::format("initval={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_epoll_create1, {"epoll_create1", [&]() {
            //int epoll_create1(int __flags);
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_dup3 (358)
        {__NR_dup3, {"dup3", [&]() {
            //int dup3(int __fd, int __fd2, int __flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("fd2={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_pipe2 (359)
        {__NR_pipe2, {"pipe2", [&]() {
            //int pipe2(int __fildes[2], int __flags);
            trace_info->fun_call->args.push_back(fmt::format("fildes[2]={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_inotify_init1 (360)
        {__NR_inotify_init1, {"inotify_init1", [&]() {
            //int inotify_init1(int __flags);
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_preadv (361)
        //__NR_pwritev (362)
        {__NR_preadv, {"preadv", [&]() {
            // ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        {__NR_pwritev, {"pwritev", [&]() {
            // ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_rt_tgsigqueueinfo (363)
        //__NR_perf_event_open (364)
        {__NR_rt_tgsigqueueinfo, {"rt_tgsigqueueinfo", [&]() {
            // int rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t *uinfo);
            trace_info->fun_call->args.push_back(fmt::format("tgid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("uinfo={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_perf_event_open 241
        {__NR_perf_event_open, {"perf_event_open", [&]() {
            // int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("cpu={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("group_fd={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_recvmmsg (365)
        {__NR_recvmmsg, {"recvmmsg", [&]() {
            // int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, struct timespec *timeout);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msgvec={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("vlen={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_accept4 (366)
        {__NR_accept4, {"accept4", [&]() {
            // int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_fanotify_init (367)
        //__NR_fanotify_mark (368)
        {__NR_fanotify_init, {"fanotify_init", [&]() {
            // int fanotify_init(unsigned int flags, unsigned int event_f_flags);
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("event_f_flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_fanotify_mark 263
        {__NR_fanotify_mark, {"fanotify_mark", [&]() {
            // int fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int dirfd, const char *pathname);
            trace_info->fun_call->args.push_back(fmt::format("fanotify_fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_prlimit64 (369)
        {__NR_prlimit64, {"prlimit64", [&]() {
            // int prlimit64(pid_t pid, int resource, const struct rlimit64 *new_limit, struct rlimit64 *old_limit);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("new_limit={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("old_limit={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_name_to_handle_at (370)
        //__NR_open_by_handle_at (371)
        //__NR_clock_adjtime (372)
        {__NR_name_to_handle_at, {"name_to_handle_at", [&]() {
            // int name_to_handle_at(int dirfd, const char *pathname, struct file_handle *handle, int *mount_id, int flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("handle={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("mount_id={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_open_by_handle_at 265
        {__NR_open_by_handle_at, {"open_by_handle_at", [&]() {
            // int open_by_handle_at(int mount_fd, struct file_handle *handle, int flags);
            trace_info->fun_call->args.push_back(fmt::format("mount_fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("handle={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_clock_adjtime 266
        {__NR_clock_adjtime, {"clock_adjtime", [&]() {
            // int clock_adjtime(clockid_t clk_id, struct timex *buf);
            trace_info->fun_call->args.push_back(fmt::format("clk_id={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_syncfs (373)
        //__NR_sendmmsg (374)
        //__NR_setns (375)
        // __NR_syncfs 267
        {__NR_syncfs, {"syncfs", [&]() {
            // int syncfs(int fd);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_setns 268
        {__NR_setns, {"setns", [&]() {
            // int setns(int fd, int nstype);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("nstype={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_sendmmsg 269
        {__NR_sendmmsg, {"sendmmsg", [&]() {
            // int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags);
            trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("msgvec={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("vlen={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_process_vm_readv (376)
        //__NR_process_vm_writev (377)
        // __NR_process_vm_readv 270
        {__NR_process_vm_readv, {"process_vm_readv", [&]() {
            // ssize_t process_vm_readv(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("local_iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("liovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("remote_iov={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("riovcnt={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_process_vm_writev 271
        {__NR_process_vm_writev, {"process_vm_writev", [&]() {
            // ssize_t process_vm_writev(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt, unsigned long flags);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("local_iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("liovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("remote_iov={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("riovcnt={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_kcmp (378)
        {__NR_kcmp, {"kcmp", [&]() {
            // int kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2);
            trace_info->fun_call->args.push_back(fmt::format("pid1={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pid2={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("idx1={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("idx2={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_finit_module (379)
        //__NR_sched_setattr (380)
        //__NR_sched_getattr (381)
        {__NR_finit_module, {"finit_module", [&]() {
            // int finit_module(int fd, const char *param_values, int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("param_values={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_sched_setattr 274
        {__NR_sched_setattr, {"sched_setattr", [&]() {
            // int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_sched_getattr 275
        {__NR_sched_getattr, {"sched_getattr", [&]() {
            // int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_renameat2 (382)
        //__NR_seccomp (383)
        //__NR_getrandom (384)
        //__NR_memfd_create (385)
        {__NR_renameat2, {"renameat2", [&]() {
            // int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("olddirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("oldpath={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("newdirfd={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("newpath={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_seccomp 277
        {__NR_seccomp, {"seccomp", [&]() {
            // int seccomp(unsigned int operation, unsigned int flags, void *args);
            trace_info->fun_call->args.push_back(fmt::format("operation={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("args={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_getrandom 278
        {__NR_getrandom, {"getrandom", [&]() {
            // ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("buflen={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_memfd_create 279
        {__NR_memfd_create, {"memfd_create", [&]() {
            // int memfd_create(const char *name, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("name={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_bpf 280
        {__NR_bpf, {"bpf", [&]() {
            // int bpf(int cmd, union bpf_attr *attr, unsigned int size);
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        //__NR_execveat (387)
        //__NR_userfaultfd (388)
        //__NR_membarrier (389)
        //__NR_mlock2 (390)
        {__NR_execveat, {"execveat", [&]() {
            // int execveat(int dirfd, const char *pathname, char *const argv[], char *const envp[], int flags);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("argv={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("envp={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_userfaultfd 282
        {__NR_userfaultfd, {"userfaultfd", [&]() {
            // int userfaultfd(int flags);
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_membarrier 283
        {__NR_membarrier, {"membarrier", [&]() {
            // int membarrier(int cmd, int flags);
            trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_mlock2 284
        {__NR_mlock2, {"mlock2", [&]() {
            // int mlock2(const void *addr, size_t length, int flags);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_copy_file_range (391)
        //__NR_preadv2 (392)
        //__NR_pwritev2 (393)
        {__NR_copy_file_range, {"copy_file_range", [&]() {
            // ssize_t copy_file_range(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("off_in={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("off_out={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg4));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_preadv2 286
        {__NR_preadv2, {"preadv2", [&]() {
            // ssize_t preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_pwritev2 287
        {__NR_pwritev2, {"pwritev2", [&]() {
            // ssize_t pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
            trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_pkey_mprotect (394)
        //__NR_pkey_alloc (395)
        //__NR_pkey_free (396)
        {__NR_pkey_mprotect, {"pkey_mprotect", [&]() {
            // int pkey_mprotect(void *addr, size_t len, int prot, int pkey);
            trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("pkey={:#x}", arg3));
            trace_info->fun_call->ret_type = kNumber;
        }}},

        // __NR_pkey_alloc 289
        {__NR_pkey_alloc, {"pkey_alloc", [&]() {
            // int pkey_alloc(unsigned int flags, unsigned int access_rights);
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("access_rights={:#x}", arg1));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        // __NR_pkey_free 290
        {__NR_pkey_free, {"pkey_free", [&]() {
            // int pkey_free(int pkey);
            trace_info->fun_call->args.push_back(fmt::format("pkey={:#x}", arg0));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        //__NR_statx (397)
        {__NR_statx, {"statx", [&]() {
            // int statx(int dirfd, const char *pathname, int flags, unsigned int mask, struct statx *statxbuf);
            trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
            trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
            trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
            trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg3));
            trace_info->fun_call->args.push_back(fmt::format("statxbuf={:#x}", arg4));
            trace_info->fun_call->ret_type = kNumber;
        }}},
        /*  {__NR_,               {"syscalls",               [&]() {
              // This is typically a placeholder for the number of syscalls.
              trace_info->fun_call->args.push_back(fmt::format("syscall_num={:#x}", arg0));
              trace_info->fun_call->ret_type = kNumber;
          }}},*/
    };
#else
    auto sys_value = trace_info->pre_status.gpr_state.x8;

    static const std::unordered_map<uint32_t, std::pair<std::string, std::function<void()>>> handlers = {
            {__NR_io_setup,               {"io_setup",               [&]() {
                //https://man7.org/linux/man-pages/man2/io_setup.2.html
                trace_info->fun_call->args.push_back(fmt::format("nr_events={}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("ctx_idp={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_io_destroy,             {"io_destroy",             [&]() {
                //https://man7.org/linux/man-pages/man2/io_destroy.2.html
                trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_io_submit,              {"io_submit",              [&]() {
                //
                trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iocbpp={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_io_cancel,              {"io_cancel",              [&]() {
                trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iocb={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("result={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_io_getevents,           {"io_getevents",           [&]() {
                trace_info->fun_call->args.push_back(fmt::format("ctx_id={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("min_nr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nr={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("events={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setxattr,               {"setxattr",               [&]() {
                // https://man7.org/linux/man-pages/man2/setxattr.2.html
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("name={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_lsetxattr,              {"lgetxattr",              [&]() {
                // https://man7.org/linux/man-pages/man2/getxattr.2.html
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_fsetxattr,              {"fsetxattr",              [&]() {
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getxattr,               {"getxattr",               [&]() {
                //ssize_t getxattr(const char* __path, const char* __name, void* __value, size_t __size)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_lgetxattr,              {"lgetxattr",              [&]() {
                //ssize_t lgetxattr(const char* __path, const char* __name, void* __value, size_t __size)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fgetxattr,              {"fgetxattr",              [&]() {
                //ssize_t fgetxattr(int __fd, const char* __name, void* __value, size_t __size)
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_listxattr,              {"listxattr",              [&]() {
                //ssize_t listxattr(const char* __path, char* __list, size_t __size)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("list={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_llistxattr,             {"llistxattr",             [&]() {
                //ssize_t llistxattr(const char* __path, char* __list, size_t __size)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("list={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_flistxattr,             {"flistxattr",             [&]() {
                //ssize_t flistxattr(int __fd, char* __list, size_t __size)
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("list={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_removexattr,            {"removexattr",            [&]() {
                //int removexattr(const char* __path, const char* __name)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_lremovexattr,           {"lremovexattr",           [&]() {
                //int lremovexattr(const char* __path, const char* __name)
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fremovexattr,           {"fremovexattr",           [&]() {
                //int fremovexattr(int __fd, const char* __name)
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg1)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getcwd,                 {"getcwd",                 [&]() {
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                trace_info->fun_call->ret_type = kString;
            }}},
            {__NR_lookup_dcookie,         {"lookup_dcookie",         [&]() {
                //
                trace_info->fun_call->args.push_back(fmt::format("cookie={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_eventfd2,               {"eventfd2",               [&]() {
                //int eventfd(unsigned int initval, int flags);
                trace_info->fun_call->args.push_back(fmt::format("initval={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_epoll_create1,          {"epoll_create1",          [&]() {
                //int epoll_create1(int __flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_epoll_ctl,              {"epoll_ctl",              [&]() {
                //int epoll_ctl(int __epfd, int __op, int __fd, struct epoll_event* __event);
                trace_info->fun_call->args.push_back(fmt::format("epfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("op={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("event={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_epoll_pwait,            {"epoll_pwait",            [&]() {
                //int epoll_pwait(int __epfd, struct epoll_event* __events, int __maxevents, int __timeout, const sigset_t* __ss);
                trace_info->fun_call->args.push_back(fmt::format("epfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("events={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("maxevents={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("ss={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_dup,                    {"dup",                    [&]() {
                //int dup(int __fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_dup3,                   {"dup3",                   [&]() {
                //int dup3(int __fd, int __fd2, int __flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("fd2={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fcntl,                  {"fcntl",                  [&]() {
                //int fcntl(int __fd, int __cmd, ...);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
                if (arg2 != 0) {
                    trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg2));
                }
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_inotify_init1,          {"inotify_init1",          [&]() {
                //int inotify_init1(int __flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_inotify_add_watch,      {"inotify_add_watch",      [&]() {
                //int inotify_add_watch(int __fd, const char* __path, uint32_t __mask);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_inotify_rm_watch,       {"inotify_rm_watch",       [&]() {
                //int inotify_rm_watch(int __fd, int __wd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("wd={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_ioctl,                  {"ioctl",                  [&]() {
                //int ioctl(int __fd, unsigned long int __request, ...);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg1));
                if (arg2 != 0) {
                    trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg2));
                }
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_ioprio_set
            {__NR_ioprio_set,             {"ioprio_set",             [&]() {
                //int ioprio_set(int __which, int __who, int __ioprio);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("ioprio={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_ioprio_get,             {"ioprio_get",             [&]() {
                //int ioprio_get(int __which, int __who);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_flock
            {__NR_flock,                  {"flock",                  [&]() {
                //int flock(int __fd, int __operation);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("operation={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_mknodat,                {"mknodat",                [&]() {
                //int mknodat(int __fd, const char* __path, mode_t __mode, dev_t __dev);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("dev={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_mkdirat,                {"mkdirat",                [&]() {
                //int mkdirat(int __fd, const char* __path, mode_t __mode);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_unlinkat,               {"unlinkat",               [&]() {
                //int unlinkat(int __fd, const char* __path, int __flag);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("flag={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_symlinkat,              {"symlinkat",              [&]() {
                //int symlinkat(const char* __oldpath, int __fd, const char* __newpath);
                trace_info->fun_call->args.push_back(
                        fmt::format("oldpath={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg1));
                trace_info->fun_call->args.push_back(
                        fmt::format("newpath={}", read_string_from_address(arg2)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_linkat,                 {"linkat",                 [&]() {
                //int linkat(int __fromfd, const char* __frompath, int __tofd,)
                trace_info->fun_call->args.push_back(fmt::format("fromfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("frompath={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("tofd={:#x}", arg2));
                trace_info->fun_call->args.push_back(
                        fmt::format("topath={}", read_string_from_address(arg3)));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            {__NR_renameat,               {"renameat",               [&]() {
                //int renameat(int __oldfd, const char* __oldpath, int __newfd, const char* __newpath);
                trace_info->fun_call->args.push_back(fmt::format("oldfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("oldpath={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("newfd={:#x}", arg2));
                trace_info->fun_call->args.push_back(
                        fmt::format("newpath={}", read_string_from_address(arg3)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_umount2
            {__NR_umount2,                {"umount2",                [&]() {
                //int umount2(const char* __target, int __flags);
                trace_info->fun_call->args.push_back(
                        fmt::format("target={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_mount,                  {"mount",                  [&]() {
                //int mount(const char* __source, const char* __target, const char* __filesystemtype, unsigned long __mountflags, const void* __data)
                trace_info->fun_call->args.push_back(
                        fmt::format("source={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("target={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(
                        fmt::format("filesystemtype={}", read_string_from_address(arg2)));
                trace_info->fun_call->args.push_back(fmt::format("mountflags={:#x}", arg3));
                trace_info->fun_call->args.push_back(
                        fmt::format("data={}", read_string_from_address(arg4)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_pivot_root,             {"pivot_root",             [&]() {
                //int pivot_root(const char* __new_root, const char* __put_old);
                trace_info->fun_call->args.push_back(
                        fmt::format("new_root={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("put_old={}", read_string_from_address(arg1)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_nfsservctl
            {__NR_nfsservctl,             {"nfsservctl",             [&]() {
                //int nfsservctl(int __fd, int __cmd, void* __argp);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("argp={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR3264_statfs
            {__NR3264_statfs,             {"statfs",                 [&]() {
                //int statfs(const char* __path, struct statfs* __buf);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR3264_fstatfs
            {__NR3264_fstatfs,            {"fstatfs",                [&]() {
                //int fstatfs(int __fd, struct statfs* __buf);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR3264_truncate
            {__NR3264_truncate,           {"truncate",               [&]() {
                //int truncate(const char* __path, long __length);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR3264_ftruncate
            {__NR3264_ftruncate,          {"ftruncate",              [&]() {
                //int ftruncate(int __fd, long __length);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_fallocate
            {__NR_fallocate,              {"fallocate",              [&]() {
                //int fallocate(int __fd, int __mode, long __offset, long __length);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_faccessat
            {__NR_faccessat,              {"faccessat",              [&]() {
                //int faccessat(int __dirfd, const char* __path, int __mode, int __flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_chdir
            {__NR_chdir,                  {"chdir",                  [&]() {
                //int chdir(const char* __path);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_fchdir
            {__NR_fchdir,                 {"fchdir",                 [&]() {
                //int fchdir(int __fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_chroot
            {__NR_chroot,                 {"chroot",                 [&]() {
                //int chroot(const char* __path);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_fchmod
            {__NR_fchmod,                 {"fchmod",                 [&]() {
                //int fchmod(int __fd, mode_t __mode);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_fchmodat
            {__NR_fchmodat,               {"fchmodat",               [&]() {
                //int fchmodat(int __dirfd, const char* __path, mode_t __mode, int __flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fchownat,               {"fchownat",               [&]() {
                //int fchownat(int __dirfd, const char* __path, uid_t __uid, gid_t __gid, int __flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fchown,                 {"fchown",                 [&]() {
                //int fchown(int __fd, uid_t __uid, gid_t __gid);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_openat
            {__NR_openat,                 {"openat",                 [&]() {
                //int openat(int __dirfd, const char* __path, int __flags, mode_t __mode);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_close
            {__NR_close,                  {"close",                  [&]() {
                //int close(int __fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_vhangup
            {__NR_vhangup,                {"vhangup",                [&]() {
                //int vhangup(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_pipe2
            {__NR_pipe2,                  {"pipe2",                  [&]() {
                //int pipe2(int __fildes[2], int __flags);
                trace_info->fun_call->args.push_back(fmt::format("fildes[2]={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_quotactl
            {__NR_quotactl,               {"quotactl",               [&]() {
                //int quotactl(int __cmd, const char* __special, int __id, void* __addr);
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("special={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("id={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg3));
            }}},
            //__NR_getdents64
            {__NR_getdents64,             {"getdents64",             [&]() {
                //int getdents64(int __fd, struct linux_dirent64* __dirp, unsigned int __count);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("dirp={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR3264_lseek
            {__NR3264_lseek,              {"lseek",                  [&]() {
                //off_t lseek(int __fd, off_t __offset, int __whence);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("whence={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_read
            {__NR_read,                   {"read",                   [&]() {
                //ssize_t read(int __fd, void* __buf, size_t __nbytes);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nbytes={:#x}", arg2));
            }}},
            {__NR_write,                  {"write",                  [&]() {
                // ssize_t write(int fd, const void *buf, size_t count);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_readv,                  {"readv",                  [&]() {
                // ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_writev,                 {"writev",                 [&]() {
                // ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_pread64,                {"pread64",                [&]() {
                // ssize_t pread64(int fd, void *buf, size_t count, off_t offset);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_pwrite64,               {"pwrite64",               [&]() {
                // ssize_t pwrite64(int fd, const void *buf, size_t count, off_t offset);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_preadv,                 {"preadv",                 [&]() {
                // ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_pwritev,                {"pwritev",                [&]() {
                // ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR3264_sendfile,           {"sendfile",               [&]() {
                // ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
                trace_info->fun_call->args.push_back(fmt::format("out_fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("in_fd={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_pselect6,               {"pselect6",               [&]() {
                // int pselect6(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec *timeout, const sigset_t *sigmask);
                trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("readfds={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("writefds={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("exceptfds={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("sigmask={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_ppoll,                  {"ppoll",                  [&]() {
                // int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask);
                trace_info->fun_call->args.push_back(fmt::format("fds={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nfds={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("sigmask={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_signalfd4,              {"signalfd4",              [&]() {
                // int signalfd4(int fd, const sigset_t *mask, size_t sizemask, int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("sizemask={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_vmsplice,               {"vmsplice",               [&]() {
                // ssize_t vmsplice(int fd, const struct iovec *iov, unsigned long nr_segs, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nr_segs={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_splice,                 {"splice",                 [&]() {
                // ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("off_in={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("off_out={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_tee,                    {"tee",                    [&]() {
                // ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_readlinkat,             {"readlinkat",             [&]() {
                // ssize_t readlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("pathname={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("bufsiz={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR3264_fstatat,            {"fstatat",                [&]() {
                // int fstatat(int dirfd, const char *pathname, struct stat *statbuf, int flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("pathname={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR3264_fstat,              {"fstat",                  [&]() {
                // int fstat(int fd, struct stat *statbuf);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("statbuf={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sync,                   {"sync",                   [&]() {
                // void sync(void);
                trace_info->fun_call->ret_type = kVoid;
            }}},
            {__NR_fsync,                  {"fsync",                  [&]() {
                // int fsync(int fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_fdatasync,              {"fdatasync",              [&]() {
                // int fdatasync(int fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sync_file_range,        {"sync_file_range",        [&]() {
                // int sync_file_range(int fd, off64_t offset, off64_t nbytes, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nbytes={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timerfd_create,         {"timerfd_create",         [&]() {
                // int timerfd_create(int clockid, int flags);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timerfd_settime,        {"timerfd_settime",        [&]() {
                // int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timerfd_gettime,        {"timerfd_gettime",        [&]() {
                // int timerfd_gettime(int fd, struct itimerspec *curr_value);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_utimensat,              {"utimensat",              [&]() {
                // int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(
                        fmt::format("pathname={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("times={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_acct,                   {"acct",                   [&]() {
                // int acct(const char *filename);
                trace_info->fun_call->args.push_back(
                        fmt::format("filename={}", read_string_from_address(arg0)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_capget,                 {"capget",                 [&]() {
                // int capget(cap_user_header_t hdrp, cap_user_data_t datap);
                trace_info->fun_call->args.push_back(fmt::format("hdrp={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("datap={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_capset,                 {"capset",                 [&]() {
                // int capset(cap_user_header_t hdrp, const cap_user_data_t datap);
                trace_info->fun_call->args.push_back(fmt::format("hdrp={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("datap={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_personality,            {"personality",            [&]() {
                // int personality(unsigned long persona);
                trace_info->fun_call->args.push_back(fmt::format("persona={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_exit,                   {"exit",                   [&]() {
                // void exit(int status);
                trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                trace_info->fun_call->ret_type = kVoid;
            }}},
            {__NR_exit_group,             {"exit_group",             [&]() {
                // void exit_group(int status);
                trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg0));
                trace_info->fun_call->ret_type = kVoid;
            }}},
            {__NR_waitid,                 {"waitid",                 [&]() {
                // int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
                trace_info->fun_call->args.push_back(fmt::format("idtype={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("id={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("infop={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("options={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_set_tid_address,        {"set_tid_address",        [&]() {
                // int set_tid_address(int *tidptr);
                trace_info->fun_call->args.push_back(fmt::format("tidptr={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_unshare,                {"unshare",                [&]() {
                // int unshare(unsigned long unshare_flags);
                trace_info->fun_call->args.push_back(fmt::format("unshare_flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_futex,                  {"futex",                  [&]() {
                // int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3);
                trace_info->fun_call->args.push_back(fmt::format("uaddr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("futex_op={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("val={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("uaddr2={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("val3={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_set_robust_list,        {"set_robust_list",        [&]() {
                // long set_robust_list(struct robust_list_head *head, size_t len);
                trace_info->fun_call->args.push_back(fmt::format("head={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_get_robust_list,        {"get_robust_list",        [&]() {
                // long get_robust_list(int pid, struct robust_list_head **head_ptr, size_t *len_ptr);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("head_ptr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len_ptr={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_nanosleep,              {"nanosleep",              [&]() {
                // int nanosleep(const struct timespec *req, struct timespec *rem);
                trace_info->fun_call->args.push_back(fmt::format("req={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("rem={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getitimer,              {"getitimer",              [&]() {
                // int getitimer(int which, struct itimerval *curr_value);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setitimer,              {"setitimer",              [&]() {
                // int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_kexec_load,             {"kexec_load",             [&]() {
                // long kexec_load(unsigned long entry, unsigned long nr_segments, struct kexec_segment *segments, unsigned long flags);
                trace_info->fun_call->args.push_back(fmt::format("entry={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nr_segments={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("segments={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_init_module,            {"init_module",            [&]() {
                // int init_module(void *module_image, unsigned long len, const char *param_values);
                trace_info->fun_call->args.push_back(fmt::format("module_image={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(
                        fmt::format("param_values={}", read_string_from_address(arg2)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_delete_module,          {"delete_module",          [&]() {
                // int delete_module(const char *name_user, unsigned int flags);
                trace_info->fun_call->args.push_back(
                        fmt::format("name_user={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timer_create,           {"timer_create",           [&]() {
                // int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sevp={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timer_gettime,          {"timer_gettime",          [&]() {
                // int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
                trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("curr_value={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timer_getoverrun,       {"timer_getoverrun",       [&]() {
                // int timer_getoverrun(timer_t timerid);
                trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timer_settime,          {"timer_settime",          [&]() {
                // int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
                trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("new_value={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("old_value={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_timer_delete,           {"timer_delete",           [&]() {
                // int timer_delete(timer_t timerid);
                trace_info->fun_call->args.push_back(fmt::format("timerid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_clock_settime,          {"clock_settime",          [&]() {
                // int clock_settime(clockid_t clockid, const struct timespec *tp);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tp={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_clock_gettime,          {"clock_gettime",          [&]() {
                // int clock_gettime(clockid_t clockid, struct timespec *tp);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tp={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_clock_getres,           {"clock_getres",           [&]() {
                // int clock_getres(clockid_t clockid, struct timespec *res);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("res={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_clock_nanosleep,        {"clock_nanosleep",        [&]() {
                // int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain);
                trace_info->fun_call->args.push_back(fmt::format("clockid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("remain={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_syslog,                 {"syslog",                 [&]() {
                // int syslog(int type, char *buf, int len);
                trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_ptrace,                 {"ptrace",                 [&]() {
                // long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);
                trace_info->fun_call->args.push_back(fmt::format("request={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("data={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_setparam,         {"sched_setparam",         [&]() {
                // int sched_setparam(pid_t pid, const struct sched_param *param);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_setscheduler,     {"sched_setscheduler",     [&]() {
                // int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_getscheduler,     {"sched_getscheduler",     [&]() {
                // int sched_getscheduler(pid_t pid);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_getparam,         {"sched_getparam",         [&]() {
                // int sched_getparam(pid_t pid, struct sched_param *param);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("param={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_setaffinity,      {"sched_setaffinity",      [&]() {
                // int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cpusetsize={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_getaffinity,      {"sched_getaffinity",      [&]() {
                // int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cpusetsize={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_yield,            {"sched_yield",            [&]() {
                // int sched_yield(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_get_priority_max, {"sched_get_priority_max", [&]() {
                // int sched_get_priority_max(int policy);
                trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_get_priority_min, {"sched_get_priority_min", [&]() {
                // int sched_get_priority_min(int policy);
                trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sched_rr_get_interval,  {"sched_rr_get_interval",  [&]() {
                // int sched_rr_get_interval(pid_t pid, struct timespec *interval);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("interval={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_restart_syscall,        {"restart_syscall",        [&]() {
                // int restart_syscall(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_kill,                   {"kill",                   [&]() {
                // int kill(pid_t pid, int sig);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_tkill,                  {"tkill",                  [&]() {
                // int tkill(int tid, int sig);
                trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_tgkill,                 {"tgkill",                 [&]() {
                // int tgkill(int tgid, int tid, int sig);
                trace_info->fun_call->args.push_back(fmt::format("tgid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sigaltstack,            {"sigaltstack",            [&]() {
                // int sigaltstack(const stack_t *ss, stack_t *old_ss);
                trace_info->fun_call->args.push_back(fmt::format("ss={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("old_ss={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigsuspend,          {"rt_sigsuspend",          [&]() {
                // int rt_sigsuspend(const sigset_t *mask, size_t sigsetsize);
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigaction,           {"rt_sigaction",           [&]() {
                // int rt_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact, size_t sigsetsize);
                trace_info->fun_call->args.push_back(fmt::format("signum={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("act={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("oldact={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigprocmask,         {"rt_sigprocmask",         [&]() {
                // int rt_sigprocmask(int how, const sigset_t *set, sigset_t *oldset, size_t sigsetsize);
                trace_info->fun_call->args.push_back(fmt::format("how={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("oldset={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigpending,          {"rt_sigpending",          [&]() {
                // int rt_sigpending(sigset_t *set, size_t sigsetsize);
                trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigtimedwait,        {"rt_sigtimedwait",        [&]() {
                // int rt_sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout, size_t sigsetsize);
                trace_info->fun_call->args.push_back(fmt::format("set={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("info={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("sigsetsize={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigqueueinfo,        {"rt_sigqueueinfo",        [&]() {
                // int rt_sigqueueinfo(pid_t pid, int sig, siginfo_t *info);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("info={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_sigreturn,           {"rt_sigreturn",           [&]() {
                // int rt_sigreturn(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setpriority,            {"setpriority",            [&]() {
                // int setpriority(int which, int who, int prio);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("prio={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getpriority,            {"getpriority",            [&]() {
                // int getpriority(int which, int who);
                trace_info->fun_call->args.push_back(fmt::format("which={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_reboot,                 {"reboot",                 [&]() {
                // int reboot(int magic, int magic2, int cmd, void *arg);
                trace_info->fun_call->args.push_back(fmt::format("magic={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("magic2={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setregid,               {"setregid",               [&]() {
                // int setregid(gid_t rgid, gid_t egid);
                trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setgid,                 {"setgid",                 [&]() {
                // int setgid(gid_t gid);
                trace_info->fun_call->args.push_back(fmt::format("gid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setreuid,               {"setreuid",               [&]() {
                // int setreuid(uid_t ruid, uid_t euid);
                trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setuid,                 {"setuid",                 [&]() {
                // int setuid(uid_t uid);
                trace_info->fun_call->args.push_back(fmt::format("uid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setresuid,              {"setresuid",              [&]() {
                // int setresuid(uid_t ruid, uid_t euid, uid_t suid);
                trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getresuid,              {"getresuid",              [&]() {
                // int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
                trace_info->fun_call->args.push_back(fmt::format("ruid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("euid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("suid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setresgid,              {"setresgid",              [&]() {
                // int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
                trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("sgid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getresgid,              {"getresgid",              [&]() {
                // int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
                trace_info->fun_call->args.push_back(fmt::format("rgid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("egid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("sgid={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setfsuid,               {"setfsuid",               [&]() {
                // int setfsuid(uid_t fsuid);
                trace_info->fun_call->args.push_back(fmt::format("fsuid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setfsgid,               {"setfsgid",               [&]() {
                // int setfsgid(gid_t fsgid);
                trace_info->fun_call->args.push_back(fmt::format("fsgid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_times,                  {"times",                  [&]() {
                // clock_t times(struct tms *buf);
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setpgid,                {"setpgid",                [&]() {
                // int setpgid(pid_t pid, pid_t pgid);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pgid={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getpgid,                {"getpgid",                [&]() {
                // pid_t getpgid(pid_t pid);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getsid,                 {"getsid",                 [&]() {
                // pid_t getsid(pid_t pid);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setsid,                 {"setsid",                 [&]() {
                // pid_t setsid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getgroups,              {"getgroups",              [&]() {
                // int getgroups(int size, gid_t list[]);
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setgroups,              {"setgroups",              [&]() {
                // int setgroups(size_t size, const gid_t *list);
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("list={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_uname,                  {"uname",                  [&]() {
                // int uname(struct utsname *buf);
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_sethostname,            {"sethostname",            [&]() {
                // int sethostname(const char *name, size_t len);
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setdomainname,          {"setdomainname",          [&]() {
                // int setdomainname(const char *name, size_t len);
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getrlimit,              {"getrlimit",              [&]() {
                // int getrlimit(int resource, struct rlimit *rlim);
                trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("rlim={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_setrlimit,              {"setrlimit",              [&]() {
                // int setrlimit(int resource, const struct rlimit *rlim);
                trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("rlim={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_getrusage,              {"getrusage",              [&]() {
                // int getrusage(int who, struct rusage *usage);
                trace_info->fun_call->args.push_back(fmt::format("who={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("usage={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_umask,                  {"umask",                  [&]() {
                // mode_t umask(mode_t mask);
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_prctl
            {__NR_prctl,                  {"prctl",                  [&]() {
                switch (arg0) {
                    case PR_SET_PDEATHSIG:
                        trace_info->fun_call->args.push_back(fmt::format("PR_SET_PDEATHSIG"));
                        trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg1));
                        trace_info->fun_call->ret_type = kNumber;
                        break;
                    case PR_GET_PDEATHSIG:
                        trace_info->fun_call->args.push_back(fmt::format("PR_GET_PDEATHSIG"));
                        trace_info->fun_call->ret_type = kNumber;
                        break;
                    case PR_SET_NAME:
                        trace_info->fun_call->args.push_back(fmt::format("PR_SET_NAME"));
                        trace_info->fun_call->args.push_back(
                                fmt::format("name={}", reinterpret_cast<const char *>(arg1)));
                        trace_info->fun_call->ret_type = kNumber;
                        break;

                    case PR_GET_NAME:
                        trace_info->fun_call->args.push_back(fmt::format("PR_GET_NAME"));
                        trace_info->fun_call->ret_type = kString;
                        break;

                    case PR_SET_SECCOMP:
                        trace_info->fun_call->args.push_back(fmt::format("PR_SET_SECCOMP"));
                        trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg1));
                        trace_info->fun_call->ret_type = kNumber;
                        break;

                    case PR_GET_SECCOMP:
                        trace_info->fun_call->args.push_back(fmt::format("PR_GET_SECCOMP"));
                        trace_info->fun_call->ret_type = kNumber;
                        break;

                    case PR_SET_NO_NEW_PRIVS:
                        trace_info->fun_call->args.push_back(fmt::format("PR_SET_NO_NEW_PRIVS"));
                        trace_info->fun_call->args.push_back(
                                fmt::format("no_new_privs={:#x}", arg1));
                        trace_info->fun_call->ret_type = kNumber;
                        break;

                    case PR_GET_NO_NEW_PRIVS:
                        trace_info->fun_call->args.push_back(fmt::format("PR_GET_NO_NEW_PRIVS"));
                        trace_info->fun_call->ret_type = kNumber;
                        break;

                    default:
                        trace_info->fun_call->args.push_back(
                                fmt::format("unknown prctl option: {:#x}", arg0));
                        trace_info->fun_call->ret_type = kNumber;
                        break;
                }
            }}},
            //__NR_getcpu
            {__NR_getcpu,                 {"getcpu",                 [&]() {
                //int getcpu(unsigned int *_Nullable cpu, unsigned int *_Nullable node);
                trace_info->fun_call->args.push_back(fmt::format("cpu={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("node={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_gettimeofday
            {__NR_gettimeofday,           {"gettimeofday",           [&]() {
                // int gettimeofday(struct timeval *tv, struct timezone *tz);
                trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_settimeofday
            {__NR_settimeofday,           {"settimeofday",           [&]() {
                // int settimeofday(const struct timeval *tv, const struct timezone *tz);
                trace_info->fun_call->args.push_back(fmt::format("tv={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tz={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_adjtimex
            {__NR_adjtimex,               {"adjtimex",               [&]() {
                // int adjtimex(struct timex *txc);
                trace_info->fun_call->args.push_back(fmt::format("txc={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_getpid
            {__NR_getpid,                 {"getpid",                 [&]() {
                // pid_t getpid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_getppid
            {__NR_getppid,                {"getppid",                [&]() {
                // pid_t getppid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_getuid
            {__NR_getuid,                 {"getuid",                 [&]() {
                // uid_t getuid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_geteuid
            {__NR_geteuid,                {"geteuid",                [&]() {
                // uid_t geteuid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_getgid
            {__NR_getgid,                 {"getgid",                 [&]() {
                // gid_t getgid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_getegid
            {__NR_getegid,                {"getegid",                [&]() {
                // gid_t getegid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_gettid
            {__NR_gettid,                 {"gettid",                 [&]() {
                // pid_t gettid(void);
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_sysinfo
            {__NR_sysinfo,                {"sysinfo",                [&]() {
                // int sysinfo(struct sysinfo *info);
                trace_info->fun_call->args.push_back(fmt::format("info={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_mq_open
            {__NR_mq_open,                {"mq_open",                [&]() {
                // mqd_t mq_open(const char *_Nonnull name, int oflag, ...);
                trace_info->fun_call->args.push_back(
                        fmt::format("name={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("oflag={}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            //__NR_mq_unlink
            {__NR_mq_unlink,              {"mq_unlink",              [&]() {
                // int mq_unlink(const char *_Nonnull name);
                trace_info->fun_call->args.push_back(
                        fmt::format("name={:#x}", read_string_from_address(arg0)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mq_timedsend 182
            {__NR_mq_timedsend,           {"mq_timedsend",           [&]() {
                // ssize_t mq_timedsend(mqd_t mqdes, const char *_Nonnull msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec *_Nonnull abs_timeout);
                trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msg_ptr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("msg_len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("msg_prio={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("abs_timeout={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mq_timedreceive 183
            {__NR_mq_timedreceive,        {"mq_timedreceive",        [&]() {
                // ssize_t mq_timedreceive(mqd_t mqdes, char *_Nonnull msg_ptr, size_t msg_len, unsigned int *_Nullable msg_prio, const struct timespec *_Nonnull abs_timeout);
                trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msg_ptr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("msg_len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("msg_prio={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("abs_timeout={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mq_notify 184
            {__NR_mq_notify,              {"mq_notify",              [&]() {
                // int mq_notify(mqd_t mqdes, const struct sigevent *_Nullable notification);
                trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("notification={:#x}", arg1));
            }}},
            // __NR_mq_getsetattr 185
            {__NR_mq_getsetattr,          {"mq_getsetattr",          [&]() {
                // int mq_getsetattr(mqd_t mqdes, const struct mq_attr *_Nonnull newattr, struct mq_attr *_Nonnull oldattr);
                trace_info->fun_call->args.push_back(fmt::format("mqdes={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("newattr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("oldattr={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_msgget 186
            {__NR_msgget,                 {"msgget",                 [&]() {
                // int msgget(key_t key, int msgflg);
                trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msgflg={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_msgctl 187
            {__NR_msgctl,                 {"msgctl",                 [&]() {
                // int msgctl(int msqid, int cmd, struct msqid_ds *_Nonnull buf);
                trace_info->fun_call->args.push_back(fmt::format("msqid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;

            }}},
            {__NR_msgrcv,                 {"msgrcv",                 [&]() {
                // ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
                trace_info->fun_call->args.push_back(fmt::format("msqid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msgp={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("msgsz={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("msgtyp={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("msgflg={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_msgsnd 189
            {__NR_msgsnd,                 {"msgsnd",                 [&]() {
                // int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
                trace_info->fun_call->args.push_back(fmt::format("msqid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msgp={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("msgsz={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("msgflg={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_semget 190
            {__NR_semget,                 {"semget",                 [&]() {
                // int semget(key_t key, int nsems, int semflg);
                trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nsems={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("semflg={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_semctl 191
            {__NR_semctl,                 {"semctl",                 [&]() {
                // int semctl(int semid, int semnum, int cmd, ...);
                trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("semnum={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg2));
                if (arg3 != 0) {
                    trace_info->fun_call->args.push_back(fmt::format("arg={:#x}", arg3));
                }
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_semtimedop 192
            {__NR_semtimedop,             {"semtimedop",             [&]() {
                // int semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout);
                trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sops={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nsops={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_semop 193
            {__NR_semop,                  {"semop",                  [&]() {
                // int semop(int semid, struct sembuf *sops, size_t nsops);
                trace_info->fun_call->args.push_back(fmt::format("semid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("sops={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("nsops={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_shmget 194
            {__NR_shmget,                 {"shmget",                 [&]() {
                // int shmget(key_t key, size_t size, int shmflg);
                trace_info->fun_call->args.push_back(fmt::format("key={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("shmflg={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_shmctl 195
            {__NR_shmctl,                 {"shmctl",                 [&]() {
                // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
                trace_info->fun_call->args.push_back(fmt::format("shmid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_shmat 196
            {__NR_shmat,                  {"shmat",                  [&]() {
                // void *shmat(int shmid, const void *shmaddr, int shmflg);
                trace_info->fun_call->args.push_back(fmt::format("shmid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("shmaddr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("shmflg={:#x}", arg2));
                trace_info->fun_call->ret_type = kPointer;
            }}},
            // __NR_shmdt 197
            {__NR_shmdt,                  {"shmdt",                  [&]() {
                // int shmdt(const void *shmaddr);
                trace_info->fun_call->args.push_back(fmt::format("shmaddr={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_socket 198
            {__NR_socket,                 {"socket",                 [&]() {
                // int socket(int domain, int type, int protocol);
                trace_info->fun_call->args.push_back(fmt::format("domain={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("protocol={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_socketpair 199
            {__NR_socketpair,             {"socketpair",             [&]() {
                // int socketpair(int domain, int type, int protocol, int sv[2]);
                trace_info->fun_call->args.push_back(fmt::format("domain={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("protocol={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("sv={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_bind 200
            {__NR_bind,                   {"bind",                   [&]() {
                // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_listen 201
            {__NR_listen,                 {"listen",                 [&]() {
                // int listen(int sockfd, int backlog);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("backlog={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_accept 202
            {__NR_accept,                 {"accept",                 [&]() {
                // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_connect 203
            {__NR_connect,                {"connect",                [&]() {
                // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_getsockname 204
            {__NR_getsockname,            {"getsockname",            [&]() {
                // int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_getpeername 205
            {__NR_getpeername,            {"getpeername",            [&]() {
                // int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_sendto 206
            {__NR_sendto,                 {"sendto",                 [&]() {
                // ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("dest_addr={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_recvfrom 207
            {__NR_recvfrom,               {"recvfrom",               [&]() {
                // ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("src_addr={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_setsockopt 208
            {__NR_setsockopt,             {"setsockopt",             [&]() {
                // int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("level={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("optname={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("optval={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("optlen={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_getsockopt 209
            {__NR_getsockopt,             {"getsockopt",             [&]() {
                // int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("level={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("optname={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("optval={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("optlen={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_shutdown 210
            {__NR_shutdown,               {"shutdown",               [&]() {
                // int shutdown(int sockfd, int how);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("how={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_sendmsg 211
            {__NR_sendmsg,                {"sendmsg",                [&]() {
                // ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msg={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_recvmsg 212
            {__NR_recvmsg,                {"recvmsg",                [&]() {
                // ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msg={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_readahead 213
            {__NR_readahead,              {"readahead",              [&]() {
                // ssize_t readahead(int fd, off64_t offset, size_t count);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("count={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_brk 214
            {__NR_brk,                    {"brk",                    [&]() {
                // int brk(void *addr);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_munmap 215
            {__NR_munmap,                 {"munmap",                 [&]() {
                // int munmap(void *addr, size_t length);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mremap 216
            {__NR_mremap,                 {"mremap",                 [&]() {
                // void *mremap(void *old_address, size_t old_size, size_t new_size, int flags, void *new_address);
                trace_info->fun_call->args.push_back(fmt::format("old_address={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("old_size={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("new_size={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("new_address={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_add_key 217
            {__NR_add_key,                {"add_key",                [&]() {
                // int add_key(const char *type, const char *description, const void *payload, size_t plen, key_serial_t destringid);
                trace_info->fun_call->args.push_back(
                        fmt::format("type={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("description={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(fmt::format("payload={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("plen={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("destringid={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;

            }}},
            // __NR_request_key 218
            {__NR_request_key,            {"request_key",            [&]() {
                // key_serial_t request_key(const char *type, const char *description, const char *callout_info, key_serial_t destringid);
                trace_info->fun_call->args.push_back(
                        fmt::format("type={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(
                        fmt::format("description={}", read_string_from_address(arg1)));
                trace_info->fun_call->args.push_back(
                        fmt::format("callout_info={}", read_string_from_address(arg2)));
                trace_info->fun_call->args.push_back(
                        fmt::format("destringid={}", read_string_from_address(arg3)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_keyctl 219
            {__NR_keyctl,                 {"keyctl",                 [&]() {
                // long keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("arg2={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("arg3={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("arg4={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("arg5={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_clone 220
            {__NR_clone,                  {"clone",                  [&]() {
                // pid_t clone(unsigned long flags, unsigned long stack_size, int signal, void *parent_tid, void *child_tid, void *tls);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("stack_size={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("signal={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("parent_tid={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("child_tid={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("tls={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_execve 221
            {__NR_execve,                 {"execve",                 [&]() {
                // int execve(const char *pathname, char *const argv[], char *const envp[]);
                trace_info->fun_call->args.push_back(
                        fmt::format("pathname={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("argv={}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("envp={}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR3264_mmap 222
            {__NR3264_mmap,               {"mmap",                   [&]() {
                // void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR3264_fadvise64 223
            {__NR3264_fadvise64,          {"fadvise64",              [&]() {
                // int fadvise64(int fd, off_t offset, size_t len, int advice);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("advice={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_swapon 224
            {__NR_swapon,                 {"swapon",                 [&]() {
                // int swapon(const char *path, int swapflags);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->args.push_back(fmt::format("swapflags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_swapoff 225
            {__NR_swapoff,                {"swapoff",                [&]() {
                // int swapoff(const char *path);
                trace_info->fun_call->args.push_back(
                        fmt::format("path={}", read_string_from_address(arg0)));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mprotect 226
            {__NR_mprotect,               {"mprotect",               [&]() {
                // int mprotect(void *addr, size_t len, int prot);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
            }}},
            // __NR_msync 227
            {__NR_msync,                  {"msync",                  [&]() {
                // int msync(void *addr, size_t len, int flags);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mlock 228
            {__NR_mlock,                  {"mlock",                  [&]() {
                // int mlock(const void *addr, size_t len);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_munlock 229
            {__NR_mlock,                  {"mlock",                  [&]() {
                // int mlock(const void *addr, size_t len);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mlockall 230
            {__NR_mlockall,               {"mlockall",               [&]() {
                // int mlockall(int flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_munlockall 231
            {__NR_mlockall,               {"mlockall",               [&]() {
                // int mlockall(int flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mincore 232
            {__NR_mincore,                {"mincore",                [&]() {
                // int mincore(void *addr, size_t len, unsigned char *vec);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("vec={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_madvise 233
            {__NR_madvise,                {"madvise",                [&]() {
                // int madvise(void *addr, size_t len, int advice);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("advice={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_remap_file_pages 234
            {__NR_remap_file_pages,       {"remap_file_pages",       [&]() {
                // int remap_file_pages(void *addr, size_t size, int prot, size_t pgoff, int flags);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("pgoff={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_mbind 235
            {__NR_mbind,                  {"mbind",                  [&]() {
                // int mbind(void *addr, size_t len, int mode, const unsigned long *nodemask, unsigned long maxnode, unsigned flags);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("nodemask={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_get_mempolicy 236
            {__NR_get_mempolicy,          {"get_mempolicy",          [&]() {
                // int get_mempolicy(int *policy, const unsigned long *nmask, unsigned long maxnode, void *addr, unsigned long flags);
                trace_info->fun_call->args.push_back(fmt::format("policy={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nmask={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_set_mempolicy 237
            {__NR_set_mempolicy,          {"set_mempolicy",          [&]() {
                // int set_mempolicy(int mode, const unsigned long *nmask, unsigned long maxnode);
                trace_info->fun_call->args.push_back(fmt::format("mode={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nmask={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_migrate_pages 238
            {__NR_migrate_pages,          {"migrate_pages",          [&]() {
                // int migrate_pages(pid_t pid, unsigned long maxnode, const unsigned long *old_nodes, const unsigned long *new_nodes);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("maxnode={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("old_nodes={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("new_nodes={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_move_pages 239
            {__NR_move_pages,             {"move_pages",             [&]() {
                // int move_pages(pid_t pid, int nr_pages, const void *pages[], const int *nodes[], int *status[], int flags);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nr_pages={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("pages={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("nodes={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            {__NR_rt_tgsigqueueinfo,      {"rt_tgsigqueueinfo",      [&]() {
                // int rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t *uinfo);
                trace_info->fun_call->args.push_back(fmt::format("tgid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("tid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("sig={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("uinfo={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_perf_event_open 241
            {__NR_perf_event_open,        {"perf_event_open",        [&]() {
                // int perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags);
                trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("cpu={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("group_fd={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_accept4 242
            {__NR_accept4,                {"accept4",                [&]() {
                // int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("addrlen={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_recvmmsg 243
            {__NR_recvmmsg,               {"recvmmsg",               [&]() {
                // int recvmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, struct timespec *timeout);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msgvec={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("vlen={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("timeout={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_arch_specific_syscall 244
            {__NR_arch_specific_syscall,  {"arch_specific_syscall",  [&]() {
                // Arch-specific syscall, parameters depend on the architecture.
                trace_info->fun_call->args.push_back(fmt::format("arg0={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("arg1={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("arg2={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("arg3={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("arg4={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("arg5={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_wait4 260
            {__NR_wait4,                  {"wait4",                  [&]() {
                // pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("status={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("options={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("rusage={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_prlimit64 261
            {__NR_prlimit64,              {"prlimit64",              [&]() {
                // int prlimit64(pid_t pid, int resource, const struct rlimit64 *new_limit, struct rlimit64 *old_limit);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("resource={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("new_limit={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("old_limit={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_fanotify_init 262
            {__NR_fanotify_init,          {"fanotify_init",          [&]() {
                // int fanotify_init(unsigned int flags, unsigned int event_f_flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("event_f_flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_fanotify_mark 263
            {__NR_fanotify_mark,          {"fanotify_mark",          [&]() {
                // int fanotify_mark(int fanotify_fd, unsigned int flags, uint64_t mask, int dirfd, const char *pathname);
                trace_info->fun_call->args.push_back(fmt::format("fanotify_fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_name_to_handle_at 264
            {__NR_name_to_handle_at,      {"name_to_handle_at",      [&]() {
                // int name_to_handle_at(int dirfd, const char *pathname, struct file_handle *handle, int *mount_id, int flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("handle={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("mount_id={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_open_by_handle_at 265
            {__NR_open_by_handle_at,      {"open_by_handle_at",      [&]() {
                // int open_by_handle_at(int mount_fd, struct file_handle *handle, int flags);
                trace_info->fun_call->args.push_back(fmt::format("mount_fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("handle={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_clock_adjtime 266
            {__NR_clock_adjtime,          {"clock_adjtime",          [&]() {
                // int clock_adjtime(clockid_t clk_id, struct timex *buf);
                trace_info->fun_call->args.push_back(fmt::format("clk_id={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_syncfs 267
            {__NR_syncfs,                 {"syncfs",                 [&]() {
                // int syncfs(int fd);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_setns 268
            {__NR_setns,                  {"setns",                  [&]() {
                // int setns(int fd, int nstype);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("nstype={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_sendmmsg 269
            {__NR_sendmmsg,               {"sendmmsg",               [&]() {
                // int sendmmsg(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags);
                trace_info->fun_call->args.push_back(fmt::format("sockfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("msgvec={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("vlen={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_process_vm_readv 270
            {__NR_process_vm_readv,       {"process_vm_readv",       [&]() {
                // ssize_t process_vm_readv(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt, unsigned long flags);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("local_iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("liovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("remote_iov={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("riovcnt={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_process_vm_writev 271
            {__NR_process_vm_writev,      {"process_vm_writev",      [&]() {
                // ssize_t process_vm_writev(pid_t pid, const struct iovec *local_iov, unsigned long liovcnt, const struct iovec *remote_iov, unsigned long riovcnt, unsigned long flags);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("local_iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("liovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("remote_iov={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("riovcnt={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_kcmp 272
            {__NR_kcmp,                   {"kcmp",                   [&]() {
                // int kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2);
                trace_info->fun_call->args.push_back(fmt::format("pid1={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pid2={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("type={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("idx1={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("idx2={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_finit_module 273
            {__NR_finit_module,           {"finit_module",           [&]() {
                // int finit_module(int fd, const char *param_values, int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("param_values={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_sched_setattr 274
            {__NR_sched_setattr,          {"sched_setattr",          [&]() {
                // int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_sched_getattr 275
            {__NR_sched_getattr,          {"sched_getattr",          [&]() {
                // int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("pid={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_renameat2 276
            {__NR_renameat2,              {"renameat2",              [&]() {
                // int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("olddirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("oldpath={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("newdirfd={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("newpath={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_seccomp 277
            {__NR_seccomp,                {"seccomp",                [&]() {
                // int seccomp(unsigned int operation, unsigned int flags, void *args);
                trace_info->fun_call->args.push_back(fmt::format("operation={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("args={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_getrandom 278
            {__NR_getrandom,              {"getrandom",              [&]() {
                // ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("buf={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("buflen={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_memfd_create 279
            {__NR_memfd_create,           {"memfd_create",           [&]() {
                // int memfd_create(const char *name, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("name={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_bpf 280
            {__NR_bpf,                    {"bpf",                    [&]() {
                // int bpf(int cmd, union bpf_attr *attr, unsigned int size);
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("attr={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("size={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_execveat 281
            {__NR_execveat,               {"execveat",               [&]() {
                // int execveat(int dirfd, const char *pathname, char *const argv[], char *const envp[], int flags);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("argv={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("envp={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_userfaultfd 282
            {__NR_userfaultfd,            {"userfaultfd",            [&]() {
                // int userfaultfd(int flags);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_membarrier 283
            {__NR_membarrier,             {"membarrier",             [&]() {
                // int membarrier(int cmd, int flags);
                trace_info->fun_call->args.push_back(fmt::format("cmd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_mlock2 284
            {__NR_mlock2,                 {"mlock2",                 [&]() {
                // int mlock2(const void *addr, size_t length, int flags);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("length={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_copy_file_range 285
            {__NR_copy_file_range,        {"copy_file_range",        [&]() {
                // ssize_t copy_file_range(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd_in={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("off_in={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("fd_out={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("off_out={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg4));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg5));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_preadv2 286
            {__NR_preadv2,                {"preadv2",                [&]() {
                // ssize_t preadv2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_pwritev2 287
            {__NR_pwritev2,               {"pwritev2",               [&]() {
                // ssize_t pwritev2(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags);
                trace_info->fun_call->args.push_back(fmt::format("fd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("iov={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("iovcnt={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("offset={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_pkey_mprotect 288
            {__NR_pkey_mprotect,          {"pkey_mprotect",          [&]() {
                // int pkey_mprotect(void *addr, size_t len, int prot, int pkey);
                trace_info->fun_call->args.push_back(fmt::format("addr={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("len={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("prot={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("pkey={:#x}", arg3));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_pkey_alloc 289
            {__NR_pkey_alloc,             {"pkey_alloc",             [&]() {
                // int pkey_alloc(unsigned int flags, unsigned int access_rights);
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("access_rights={:#x}", arg1));
                trace_info->fun_call->ret_type = kNumber;
            }}},
            // __NR_pkey_free 290
            {__NR_pkey_free,              {"pkey_free",              [&]() {
                // int pkey_free(int pkey);
                trace_info->fun_call->args.push_back(fmt::format("pkey={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_statx 291
            {__NR_statx,                  {"statx",                  [&]() {
                // int statx(int dirfd, const char *pathname, int flags, unsigned int mask, struct statx *statxbuf);
                trace_info->fun_call->args.push_back(fmt::format("dirfd={:#x}", arg0));
                trace_info->fun_call->args.push_back(fmt::format("pathname={:#x}", arg1));
                trace_info->fun_call->args.push_back(fmt::format("flags={:#x}", arg2));
                trace_info->fun_call->args.push_back(fmt::format("mask={:#x}", arg3));
                trace_info->fun_call->args.push_back(fmt::format("statxbuf={:#x}", arg4));
                trace_info->fun_call->ret_type = kNumber;
            }}},

            // __NR_syscalls 292
            {__NR_syscalls,               {"syscalls",               [&]() {
                // This is typically a placeholder for the number of syscalls.
                trace_info->fun_call->args.push_back(fmt::format("syscall_num={:#x}", arg0));
                trace_info->fun_call->ret_type = kNumber;
            }}},
    };
#endif
    auto it = handlers.find(sys_value);
    if (it != handlers.end()) {
        trace_info->fun_call->fun_name = it->second.first;
        trace_info->fun_call->args.clear();
        it->second.second();
        return true;
    } else {
        trace_info->fun_call->fun_name = "unknown";
        trace_info->fun_call->args.clear();
        trace_info->fun_call->ret_type = kNumber;
    }
    return false;
}

bool DispatchSyscall::dispatch_ret(inst_trace_info_t *info, const QBDI::GPRState *ret_status) {
    auto ret_value = get_ret_register_value(ret_status, 0);
    record_memory_info(info, ret_status);
    info->fun_call->ret_value = fmt::format("ret= {:#x}", ret_value);
    return true;
}

void DispatchSyscall::record_memory_info(inst_trace_info_t *trace_info,
                                         const QBDI::GPRState *ret_status) {
    auto fun_name = trace_info->fun_call->fun_name;
    if (fun_name.empty()) {
        return;
    }
    auto ret_value = get_ret_register_value(ret_status, 0);
    auto arg0 = get_arg_register_value(&trace_info->pre_status, 0);
    auto arg1 = get_arg_register_value(&trace_info->pre_status, 1);
    auto arg2 = get_arg_register_value(&trace_info->pre_status, 2);
    const std::unordered_map<std::string, std::function<void()>> memory_fun_handlers = {
            {"mmap",   [&]() {
                //void* mmap(void* __addr, size_t __byte_count, int __prot, int __flags, int __fd, off_t __offset)
                if (arg1 == 0 || ret_value == 0) {
                    return;
                }
                trace_info->fun_call->memory_alloc_address = ret_value;
                trace_info->fun_call->memory_alloc_size = arg1;
            }},
            {"mmap2",  [&]() {
                //void* mmap(void* __addr, size_t __byte_count, int __prot, int __flags, int __fd, off_t __offset)
                if (arg1 == 0 || ret_value == 0) {
                    return;
                }
                trace_info->fun_call->memory_alloc_address = ret_value;
                trace_info->fun_call->memory_alloc_size = arg1;
            }},
            {"munmap", [&]() {
                //int munmap(void* __addr, size_t __byte_count)
                if (arg0 == 0) {
                    return;
                }
                trace_info->fun_call->memory_free_address = arg0;
            }},
    };
    if (memory_fun_handlers.find(fun_name) != memory_fun_handlers.end()) {
        memory_fun_handlers.at(fun_name)();
    }
}

DispatchSyscall::DispatchSyscall() {}