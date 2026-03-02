"""PlatformIO extra script: load .env file into build flags."""
import os

Import("env")

env_file = os.path.join(env.subst("$PROJECT_DIR"), ".env")
if os.path.isfile(env_file):
    with open(env_file) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, val = line.split("=", 1)
            key, val = key.strip(), val.strip()
            env.Append(CPPDEFINES=[(key, env.StringifyMacro(val))])
