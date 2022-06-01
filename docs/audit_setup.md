# System Auditing Setup
## Install Linux Kernel (Optional):
We develop a new kernel based on [Linux 4.15](https://github.com/torvalds/linux/tree/v4.15) to support recording more
attributes of system entities (e.g., version for files) than 
[the default Linux Audit
Framework](https://github.com/linux-audit/audit-kernel).
```bash
wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1-RMonFZ0NyRLdnwyVoB7HziaODesOg-7' -O linux-image-4.15.0_1.0.side.information_amd64.deb
sudo dpkg -i linux-image-4.15.0_1.0.side.information_amd64.deb
(Reboot Ubuntu with Linux 4.15.0 kernel)
```
## Install auditbeat developed by [Elastic](https://www.elastic.co/beats/auditbeat):
Note: auditbeat depends on auditd.
```bash
sudo apt install auditd
sudo apt install auditbeat
sudo service auditd stop
sudo service auditbeat stop
sudo systemctl disable auditd
sudo systemctl disable auditbeat
```
