import os
import subprocess

API_GEN_DIR = '.'
PY_OUT      = '../py/project/api'
H_OUT       = '../fw/app/mcu/inc/api'
C_OUT       = '../fw/app/mcu/src/api'
MAP_FILE    = 'api_map.py'

OUT_DIRS = [PY_OUT, H_OUT, C_OUT]

def create_dirs():
    for d in OUT_DIRS:
        os.makedirs(d, exist_ok=True)
        print(f"Directorio creado: {d}")

def run_generate():
    cmd = ['python', os.path.join(API_GEN_DIR, 'generate.py'), '-i', MAP_FILE]
    print("Ejecutando:", ' '.join(cmd))
    subprocess.run(cmd, check=True)

def clean():
    build_file = os.path.join('out', 'build')
    if os.path.exists(build_file):
        os.remove(build_file)
        print("Eliminado:", build_file)
    else:
        print("No existe:", build_file)

if __name__ == "__main__":
    import sys
    if len(sys.argv) == 1 or sys.argv[1] == 'all':
        create_dirs()
        run_generate()
    elif sys.argv[1] == 'clean':
        clean()
    else:
        print("Opciones: all | clean")