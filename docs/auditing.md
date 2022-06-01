# System Auditing
Once auditbeat is installed, you can collect audit records by
1. Configuring auditbeat to start system auditing (save machine meta-information in the `test` folder):
```bash
cd audit
sudo bash config.sh test
```
2. Stopping system auditing and collecting audit data into the `test` folder:
```bash
sudo bash collect.sh test
```

Note: ShadeWatcher currently support recording 32 types of system calls: read, write, open, close, mq_open, openat, unlink, link, linkat, unlinkat, rmdir, mkdir, rename, pipe, pipe2, dup, dup2, fcntl, clone, fork, vfork, execve, kill, sendto, recvfrom, sendmsg, sendmmsg, recvmsg, recvmmsg, connect, socket, and getpeername.

## Understand Auditing's Output
* `auditbeat.x` - Describes audit records in a json format
* `procinfo` - Contains meta-information (e.g., pid, ppid and arguments) of
  existing processes before system auditing
* `fdinfo` - Contains opened files of existing processes before system auditing
* `socketinfo` - Contains existing sockets before system auditing

## Example of an audit record
A log entry of `read` is shown as follows:
```json
{
  "@timestamp": "2020-10-31T13:18:31.013Z",
  "@metadata": {
    "beat": "auditbeat",
    "type": "doc",
    "version": "6.8.12"
  },
  "user": {
    "name_map": {
      "fsgid": "anonymized",
      "fsuid": "anonymized",
      "gid": "anonymized",
      "sgid": "anonymized",
      "uid": "anonymized",
      "auid": "anonymized",
      "egid": "anonymized",
      "euid": "anonymized",
      "suid": "anonymized"
    },
    "egid": "1000",
    "fsgid": "1000",
    "auid": "1000",
    "fsuid": "1000",
    "gid": "1000",
    "sgid": "1000",
    "euid": "1000",
    "suid": "1000",
    "uid": "1000"
  },
  "process": {
    "ppid": "14873",
    "name": "sshd",
    "exe": "/usr/sbin/sshd",
    "pid": "14936"
  },
  "auditd": {
    "result": "success",
    "session": "697",
    "data": {
      "a3": "7ffcd033f420",
      "syscall": "read",
      "tty": "(none)",
      "a2": "4000",
      "exit": "36",
      "arch": "x86_64",
      "a0": "3",
      "a1": "7ffcd033b490"
    },
    "sequence": 29541
  }
}
```
