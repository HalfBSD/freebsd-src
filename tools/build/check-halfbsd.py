#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Check HalfBSD build selections without compiling or touching object trees."""

import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--make", default=shutil.which("bmake") or shutil.which("make"))
    args = parser.parse_args()
    if not args.make:
        parser.error("BSD make is required; specify --make")
    root = Path(__file__).resolve().parents[2]
    common = [
        args.make, "-r", "-m", str(root / "share/mk"),
        "SRCTOP=" + str(root), "MACHINE=amd64", "MACHINE_ARCH=amd64",
        "MACHINE_CPUARCH=amd64", "TARGET=amd64", "TARGET_ARCH=amd64",
        "COMPILER_TYPE=clang", "COMPILER_VERSION=180000",
        "COMPILER_FREEBSD_VERSION=0", "LINKER_TYPE=lld", "LINKER_VERSION=180000",
        "MK_AUTO_OBJ=no", "MK_SYSROOT=no", "MK_DIRDEPS_BUILD=no",
        "MK_META_MODE=no", "MK_STAGING=no", ".MAKE.MODE=normal",
    ]
    with tempfile.TemporaryDirectory(prefix="halfbsd-policy-") as tmp:
        probe = Path(tmp) / "probe.mk"

        def evaluate(include, variables, overrides=(), cwd=root, succeeds=True):
            probe.write_text('.include "' + str(root / include) + '"\n')
            cmd = common + ["-f", str(probe)] + list(overrides)
            for variable in variables:
                cmd += ["-V", "${" + variable + "}"]
            result = subprocess.run(cmd, cwd=cwd, text=True, capture_output=True)
            if succeeds:
                assert result.returncode == 0, result.stderr
            else:
                assert result.returncode != 0, "unsupported policy override accepted"
            return result.stdout.splitlines(), result.stderr

        opts = "share/mk/src.opts.mk"
        bootstrap = root / "tools/build"
        for host_os in ["FreeBSD", "Linux"]:
            result = subprocess.run(
                common + ["-C", str(bootstrap), ".MAKE.OS=" + host_os,
                          "CC=cc", "CXX=c++", "CPP=cpp", "LD=ld",
                          "DESTDIR=" + tmp + "/legacy", "-n",
                          "installdirs", "host-symlinks"],
                text=True, capture_output=True)
            assert result.returncode == 0, result.stderr
            assert "Linking host tools" in result.stdout, result.stdout
            assert "mkdir -p " + tmp + "/legacy/bin" in result.stdout, result.stdout
        values, _ = evaluate("tools/build/Makefile",
                             ["INCS", "SYSINCS", "INCSGROUPS"],
                             ["CC=cc", "CXX=c++", "CPP=cpp", "LD=ld"], cwd=bootstrap)
        assert "mpool.h" in values[0] and "elf_common.h" in values[1], values
        assert "RPCINCS" not in values[2], values
        print("PASS: bootstrap directory/host-tool targets and non-RPC headers")
        llvm_targets = ["MK_LLVM_TARGET_" + target for target in
                        ["ALL", "X86", "AARCH64", "ARM", "RISCV", "BPF", "MIPS"]]
        values, _ = evaluate(opts, llvm_targets)
        assert values == ["no", "yes", "no", "no", "no", "no", "no"], values
        values, _ = evaluate(opts, llvm_targets, ["WITH_LLVM_TARGET_ALL=yes"])
        assert values == ["yes", "yes", "yes", "yes", "yes", "no", "no"], values
        values, _ = evaluate(opts, llvm_targets, ["WITH_LLVM_TARGET_RISCV=yes"])
        assert values == ["no", "yes", "no", "no", "yes", "no", "no"], values
        print("PASS: native LLVM backend default and explicit target overrides")
        required = ["BHYVE", "BOOT", "CDDL", "CRYPT", "EFI", "INET", "INET6",
                    "IPFW", "JAIL", "OPENSSH", "OPENSSL", "PAM", "SOUND", "USB",
                    "WIRELESS", "ZFS", "LOADER_LUA", "LOADER_ZFS"]
        values, _ = evaluate(opts, ["MK_" + opt for opt in required])
        assert values == ["yes"] * len(required), values
        values, _ = evaluate(opts, ["MK_ZFS"], ["WITHOUT_ZFS=yes"])
        assert values == ["yes"]
        _, error = evaluate(opts, ["MK_ZFS"], ["MK_ZFS=no"], succeeds=False)
        assert "HalfBSD requires MK_ZFS=yes" in error
        values, _ = evaluate(opts, ["MK_ZFS", "MK_OPENSSH"],
                             ["BOOTSTRAPPING=0", "MK_ZFS=no", "MK_OPENSSH=no"])
        assert values == ["no", "no"], values
        values, _ = evaluate(opts, ["MK_BOOT"], ["MK_BOOT=no"], cwd=root / "lib/libc")
        assert values == ["no"], values
        values, _ = evaluate(opts, ["MK_ASAN", "MK_UBSAN"], ["WITH_ASAN=yes", "WITH_UBSAN=yes"])
        assert values == ["yes", "yes"], values
        print("PASS: fixed runtime features, bootstrap/component overrides and sanitizers")

        top = "sys/modules/Makefile"
        smp = []
        values, _ = evaluate(top, ["SUBDIR"], smp)
        selected = set(" ".join(values).split())
        essential = {"vmm", "ctl", "zfs", "nvme", "if_epair", "if_bridge",
                     "linuxkpi", "linuxkpi_wlan", "linuxkpi_video", "usb", "sound",
                     "cam", "mps", "mpr", "mpi3mr"}
        assert essential <= selected, essential - selected
        zfs_core = root / "sys/contrib/openzfs/module/os/freebsd/zfs/kmod_core.c"
        zfs_dependencies = set(re.findall(
            r"MODULE_DEPEND\(zfsctrl,\s*(\w+),", zfs_core.read_text()))
        assert zfs_dependencies, "no ZFS module dependencies found"
        assert zfs_dependencies <= selected, zfs_dependencies - selected
        retired = {"ena", "gve", "mana", "vmware", "iscsi", "cfiscsi", "nvmf", "krpc",
                   "le", "dc", "fxp", "rl", "sis", "ste", "xl", "ipw", "iwi",
                   "wpi", "malo", "ipwfw", "iwifw", "wpifw"}
        assert not selected & retired
        values, _ = evaluate(top, ["SUBDIR"], smp + ["MODULES_EXTRA=accf_data"])
        assert "accf_data" in " ".join(values).split()
        values, _ = evaluate(top, ["SUBDIR"], smp + ["MODULES_OVERRIDE=virtio vmm"])
        assert set(" ".join(values).split()) == {"virtio", "vmm"}
        values, _ = evaluate(top, ["SUBDIR"], smp + ["ALL_MODULES=yes"])
        all_modules = set(" ".join(values).split())
        assert {"accf_data"} <= all_modules
        assert not retired & all_modules
        assert all((root / "sys/modules" / name / "Makefile").is_file()
                   for name in all_modules)
        values, _ = evaluate(top, ["SUBDIR"], smp + ["WITHOUT_MODULES=vmm"])
        assert "vmm" not in " ".join(values).split()
        for group, includes in [
            ("geom", {"geom_eli", "geom_mirror", "geom_part", "geom_label"}),
            ("usb", {"xhci", "umass", "usbhid", "ukbd", "ure", "runfw"}),
            ("sound/driver", {"hda", "uaudio", "driver", "dummy"}),
        ]:
            values, _ = evaluate("sys/modules/" + group + "/Makefile", ["SUBDIR"])
            names = set(" ".join(values).split())
            assert includes <= names, (group, includes - names)
            assert all((root / "sys/modules" / group / name / "Makefile").is_file() for name in names)
        retired_audio = {"ich", "via82c686", "via8233", "neomagic", "solo",
                         "t4dwave", "vibes"}
        for controls in [[], ["ALL_MODULES=yes"],
                         ["ALL_MODULES=yes", "MK_SOURCELESS_UCODE=no"]]:
            values, _ = evaluate("sys/modules/sound/driver/Makefile", ["SUBDIR"], controls)
            audio = set(" ".join(values).split())
            assert {"hda", "uaudio", "dummy", "hdsp", "hdspe"} <= audio
            assert not audio & retired_audio
            assert all((root / "sys/modules/sound/driver" / name / "Makefile").is_file()
                       for name in audio)
        retired_usb = {"g_audio", "g_keyboard", "g_modem", "g_mouse", "template",
                       "cfumass", "usfs", "uipaq", "uvisor", "urio"}
        for controls in [[], ["ALL_MODULES=yes"],
                         ["ALL_MODULES=yes", "MK_SOURCELESS_UCODE=no"]]:
            values, _ = evaluate("sys/modules/usb/Makefile", ["SUBDIR"], controls)
            usb = set(" ".join(values).split())
            assert {"xhci", "umass", "ukbd", "ums", "ucom", "uftdi", "umodem",
                    "ugensa", "ufoma", "uvscom", "ipheth", "ure"} <= usb
            assert not usb & retired_usb
            assert all((root / "sys/modules/usb" / name / "Makefile").is_file()
                       for name in usb)
        values, _ = evaluate("sys/modules/usb/Makefile", ["SUBDIR"], ["MK_SOURCELESS_UCODE=no"])
        assert not {"runfw", "rsufw"} & set(" ".join(values).split())
        assert all((root / "sys/modules" / name / "Makefile").is_file() for name in selected)
        for makefile, retained, removed in [
            ("sys/modules/cam/Makefile",
             {"scsi_da.c", "scsi_cd.c", "scsi_pass.c", "scsi_enc_ses.c", "nvme_da.c"},
             {"scsi_sa.c", "scsi_ch.c", "opt_sa.h"}),
            ("lib/libcam/Makefile", {"camlib.c", "scsi_all.c", "scsi_da.c", "nvme_all.c"},
             {"scsi_sa.c"}),
        ]:
            values, _ = evaluate(makefile, ["SRCS"], ["CC=clang", "LD=ld"])
            sources = set(" ".join(values).split())
            assert retained <= sources, (makefile, retained - sources)
            assert not sources & removed, (makefile, sources & removed)
        values, _ = evaluate("usr.sbin/camdd/Makefile", ["LIBADD"], ["CC=clang", "LD=ld"])
        assert "mt" not in " ".join(values).split()
        print("PASS: workstation modules, bhyve/CTL/ZFS, firmware selection and module overrides")

    for path in root.rglob("Makefile.depend*"):
        for line in path.read_text().splitlines():
            match = re.fullmatch(r"\s+([\w./+-]+)\s*\\?", line)
            if not match or "/" not in match[1]:
                continue
            source = re.sub(r"\.(?:host(?:32)?|amd64|i386|arm64|aarch64|armv[67])$", "", match[1])
            assert (root / source / "Makefile").is_file(), (path.relative_to(root), source)
    for variant in ["HALFBSD", "HALFBSD-DEBUG", "HALFBSD-KASAN", "HALFBSD-KCSAN", "HALFBSD-KMSAN"]:
        assert (root / "sys/amd64/conf" / variant).is_file()
    kernel = (root / "sys/amd64/conf/HALFBSD").read_text()
    devices = set(re.findall(r"^device\s+(\w+)", kernel, re.M))
    options = set(re.findall(r"^options\s+(\w+)", kernel, re.M))
    assert {"nvme", "nda", "scbus", "da", "ahci", "wlan", "iflib"} <= devices
    assert {"SMP", "VIMAGE", "INET", "INET6", "COMPAT_LINUXKPI"} <= options
    known_options = set()
    for option_file in (root / "sys/conf").glob("options*"):
        for line in option_file.read_text().splitlines():
            if line and not line.startswith("#"):
                known_options.add(line.split()[0])
    assert options <= known_options, options - known_options
    for name in ["WITHOUT_GAMES", "WITHOUT_FINGER", "WITHOUT_MODULE_DRM", "WITHOUT_ISCSI", "WITHOUT_ROUTED"]:
        assert not (root / "tools/build/options" / name).exists()
    print("PASS: committed dependency paths, maintained kernel variants and retired options")


if __name__ == "__main__":
    main()
