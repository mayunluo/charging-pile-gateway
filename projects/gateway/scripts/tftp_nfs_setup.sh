#!/bin/bash
# quick setup notes for Ubuntu host
sudo apt-get update
sudo apt-get install -y tftpd-hpa nfs-kernel-server
echo "Place zImage in /srv/tftp and export NFS rootfs in /srv/nfs"
# You still need to edit /etc/default/tftpd-hpa and /etc/exports manually per your env
