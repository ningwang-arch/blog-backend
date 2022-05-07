#include "daemon.h"

#include <sstream>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "pico/logging.h"

const uint64_t daemon_restart_interval = 60;

std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_id=" << parent_id << " main_id=" << main_id
       << " parent_start_time=" << pico::Time2Str(parent_start_time)
       << " main_start_time=" << pico::Time2Str(main_start_time)
       << " restart_count=" << restart_count << "]";
    return ss.str();
}

static int real_start(int argc, const char* argv[],
                      std::function<int(int argc, const char* argv[])> main_cb) {
    ProcessInfoMgr::getInstance()->main_id = getpid();
    ProcessInfoMgr::getInstance()->main_start_time = time(0);
    return main_cb(argc, argv);
}

static int real_daemon(int argc, const char* argv[],
                       std::function<int(int argc, const char* argv[])> main_cb) {
    int ret = daemon(1, 0);
    if (ret) {}
    ProcessInfoMgr::getInstance()->parent_id = getpid();
    ProcessInfoMgr::getInstance()->parent_start_time = time(0);
    while (true) {
        pid_t pid = fork();
        if (pid == 0) {
            //子进程返回
            ProcessInfoMgr::getInstance()->main_id = getpid();
            ProcessInfoMgr::getInstance()->main_start_time = time(0);
            LOG_INFO("progress start pid = %ld", getpid());
            return real_start(argc, argv, main_cb);
        }
        else if (pid < 0) {
            LOG_ERROR("fork fail return = %d, errno = %d, %s", pid, errno, strerror(errno));
            return -1;
        }
        else {
            //父进程返回
            int status = 0;
            waitpid(pid, &status, 0);
            if (status) {
                if (status == 9) {
                    LOG_INFO("killed");
                    break;
                }
                else {
                    LOG_ERROR("child crash pid=%d, status = %d", pid, status);
                }
            }
            else {
                LOG_INFO("child exit pid=%d", pid);
                break;
            }
            ProcessInfoMgr::getInstance()->restart_count += 1;
            sleep(daemon_restart_interval);
        }
    }
    return 0;
}

int start_daemon(int argc, const char* argv[],
                 std::function<int(int argc, const char* argv[])> main_cb, bool is_daemon) {
    if (!is_daemon) {
        ProcessInfoMgr::getInstance()->parent_id = getpid();
        ProcessInfoMgr::getInstance()->parent_start_time = time(0);
        return real_start(argc, argv, main_cb);
    }
    return real_daemon(argc, argv, main_cb);
}