import subprocess
from datetime import datetime

Import("env")

print("-" * 80)

# Tenta pegar o hash curto do commit do git
try:
    commit = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).strip().decode('utf-8')
except:
    commit = "no-git"

# Pega a data atual
date = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

# Cria a string de versão (Commit + Data)
version_string = f"{commit} ({date})"

# Define a flag de compilação -DVERSION="..."

env.Append(CPPDEFINES=[
    ("BUILD_VERSION", f"\\\"{version_string}\\\"")
])

print(f" - BUILD_VERSION: {version_string}")

print("-" * 80)