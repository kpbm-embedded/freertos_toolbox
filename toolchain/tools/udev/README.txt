Install (needs root):  sudo cp toolchain/tools/udev/*.rules /etc/udev/rules.d/
                       sudo udevadm control --reload && sudo udevadm trigger
stlink rules from stlink v1.9.0 (49-*.rules); 69-probe-rs.rules from probe-rs/webpage (covers J-Link, CMSIS-DAP, ST-LINK, WCH-Link too).
