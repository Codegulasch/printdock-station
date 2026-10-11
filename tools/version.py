# Firmware-Version: aus der Umgebung (CI: Tag „v0.2.0“ → FW_VERSION=0.2.0), sonst aus CHANGELOG.md
Import("env")  # noqa: F821
import os
import re

version = os.environ.get("FW_VERSION", "").lstrip("v")
if not version:
    with open(os.path.join(env.subst("$PROJECT_DIR"), "CHANGELOG.md"), encoding="utf-8") as f:  # noqa: F821
        m = re.search(r"^## (\d+\.\d+\.\d+)", f.read(), re.M)
        version = m.group(1) if m else "dev"
env.Append(CPPDEFINES=[("FW_VERSION", env.StringifyMacro(version))])  # noqa: F821
