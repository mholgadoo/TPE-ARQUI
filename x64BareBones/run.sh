qemu-system-x86_64 \
  -hda Image/x64BareBonesImage.qcow2 \
  -m 512 \
  -machine pcspk-audiodev=snd0 \
  -audiodev pa,id=snd0