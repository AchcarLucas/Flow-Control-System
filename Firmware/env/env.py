import os
import sys

Import("env")

print("-" * 80)

# Caminho para o seu .env (sem a barra inicial no join)
env_path = os.path.join(env.get("PROJECT_DIR"), "env", ".env")

# Verificação da existência do arquivo
print(f"Searching .env file: {env_path}")
if not os.path.exists(env_path):
    print(f" - WARNING: Environment file  {env_path} not found!")
    sys.exit(1)

# Lógica simples de uma linha para ler e injetar as variáveis
print(f"Settings CPP DEFINES with .env file")
with open(env_path) as f:
    for line in f:
        line = line.strip()
        if "=" in line and not line.startswith("#"):
            key, value = line.split("=", 1)
            value = value.strip().strip('"').strip("'")

            print(f" - {key}: {value}")
            
            # Forma robusta de passar string para o GCC:
            # O compilador espera -D KEY='"VALUE"'
            env.Append(CPPDEFINES=[
                (key, f'\\"{value}\\"')
            ])

print("-" * 80)