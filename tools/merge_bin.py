# PlatformIO-Nachbearbeitung: ein einziges Image (Bootloader + Partitionen + App) erzeugen,
# das sich ab Adresse 0x0 flashen lässt – z. B. im Browser über https://web.esphome.io
Import("env")  # noqa: F821  (von PlatformIO bereitgestellt)
import os


def merge(source, target, env):
    build = env.subst("$BUILD_DIR")
    out = os.path.join(build, "printdock-station-%s.factory.bin" % env.subst("$PIOENV"))
    esptool = os.path.join(env.PioPlatform().get_package_dir("tool-esptoolpy"), "esptool.py")
    board = env.BoardConfig()
    args = [
        '"%s"' % env.subst("$PYTHONEXE"), '"%s"' % esptool,
        "--chip", board.get("build.mcu", "esp32"), "merge_bin", "-o", '"%s"' % out,
        "--flash_mode", board.get("build.flash_mode", "dio"),
        "--flash_size", board.get("upload.flash_size", "4MB"),
    ]
    for addr, path in env.get("FLASH_EXTRA_IMAGES", []):
        args += [addr, '"%s"' % env.subst(path)]
    args += [env.subst("$ESP32_APP_OFFSET") or "0x10000", '"%s"' % os.path.join(build, "firmware.bin")]
    print("Factory-Image: %s" % out)
    env.Execute(" ".join(args))


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge)  # noqa: F821
