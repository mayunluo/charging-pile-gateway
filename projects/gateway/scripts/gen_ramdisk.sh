#!/bin/bash
set -e
OUTDIR=ramdisk_root
SYSROOT=${SYSROOT:-/usr/}
BUSYBOX_BIN=../thirdparty/busybox/busybox

rm -rf ${OUTDIR}
mkdir -p ${OUTDIR}/{bin,sbin,etc,proc,sys,usr,dev,lib,tmp,var,root}
chmod 1777 ${OUTDIR}/tmp

# copy busybox (if you have built one), or use system busybox
if [ -x "${BUSYBOX_BIN}" ]; then
  cp ${BUSYBOX_BIN} ${OUTDIR}/bin/busybox
  chroot ${OUTDIR} /bin/busybox --install -s /bin || true
else
  echo "Warning: busybox binary not found at ${BUSYBOX_BIN}. Expect failure if busybox not present."
fi

# copy minimal required libraries from sysroot (edit paths for your toolchain)
if [ -d "${SYSROOT}/lib" ]; then
  cp -L ${SYSROOT}/lib/ld-linux-armhf.so.3 ${OUTDIR}/lib/ || true
  cp -L ${SYSROOT}/lib/libm.so.6 ${OUTDIR}/lib/  || true
  cp -L ${SYSROOT}/lib/libc.so.6 ${OUTDIR}/lib/  || true
fi

# init script
cat > ${OUTDIR}/init <<'INIT'
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
echo "Starting gatewayd..."
/bin/gatewayd
INIT
chmod +x ${OUTDIR}/init

# create device nodes (console)
sudo mknod -m 666 ${OUTDIR}/dev/console c 5 1 || true

( cd ${OUTDIR} && find . | cpio -o -H newc | gzip -9 > ../initramfs.cpio.gz )
echo "Generated initramfs.cpio.gz"
