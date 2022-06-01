# Bash script to config auditbeat in Ubuntu 16.04

# !/bin/bash

echo "config auditbeat.yml"
cp auditbeat.yml /etc/auditbeat

echo "config audit-rules.conf"
audit_rule_path=/etc/auditbeat/audit.rules.d/audit-rules.conf
echo -e "-a never,exit -S all -F subj_type=pulseaudio_t -F subj_type=ntpd_t -F subj_type=cron_t\n"  > $audit_rule_path
echo -e "## Cron jobs fill the logs with stuff we normally don't want (works with SELinux) \n-a never,user -F subj_type=crond_t\n-a exit,never -F subj_type=crond_t\n-a never,user -F subj_type=cron_t\n-a exit,never -F subj_type=cron_t" >> $audit_rule_path

# Filter noisy files for auditing
echo "$(ls /home)" | python file_filter.py  >> $audit_rule_path
echo -e "-a never,exit -F dir=/sys/dev"  >> $audit_rule_path
echo -e "-a never,exit -F dir=/usr/share"  >> $audit_rule_path
echo -e "-a never,exit -F dir=/etc/fonts" >> $audit_rule_path
echo -e "-a never,exit -F dir=/var/cache" >> $audit_rule_path
echo -e "-a never,exit -F dir=/root/.cache" >> $audit_rule_path
echo -e "-a never,exit -F dir=/proc\n" >> $audit_rule_path
echo -e "-a never,exit -F auid=4294967295" >> $audit_rule_path

# Filter noisy proccesses for auditing
ps -ef | grep -E 'gnome|xorg|ibus|vmtool|gvfs|nautilus|dbus|avahi|zeitgeist|compiz|NetworkManager|x86_64-linux-gnu|upstart|bash' | grep -v 'gnome-terminal' | awk '{print $2}' | python proc_filter.py >> $audit_rule_path

# List (32) system calls for auditing
echo -e "\n-a always,exit -S read,write,open,close,clone,fork,vfork,execve,kill,mq_open,openat,sendto,recvfrom,sendfile,sendmsg,sendmmsg,recvmsg,recvmmsg,connect,socket,unlink,link,linkat,unlinkat,rmdir,mkdir,rename,pipe,pipe2,dup,dup2,getpeername,fcntl" >> $audit_rule_path

# delete previous audit records
echo "delete existing audit records"
rm /var/log/auditbeat/*

# Create a new folder to record init information
echo "collect meta-information before system auditing"
folder=$1
mkdir $folder
cd $folder

# Collect information for current processes running on the system 
mkdir procinfo
cd procinfo
ps -ef > general.txt
ps -eo pid > pid.txt
ps -eo comm > exe.txt
ps -eo args > args.txt
ps -eo ppid > ppid.txt

# Collect information for file descriptor
cd ../
mkdir fdinfo
cd fdinfo
for proc in $(ls /proc | grep '[0-9+]'); do
		touch $proc
		ls -la /proc/$proc/fd > $proc
done

# Collect information for socket descriptor
cd ../
mkdir socketinfo
cd socketinfo
lsof -i -n -P > general.txt
cat general.txt | awk '{print $6}' > device.txt
cat general.txt | awk '{print $9}' > name.txt

# Start auditbeat
service auditbeat restart
cd ../
