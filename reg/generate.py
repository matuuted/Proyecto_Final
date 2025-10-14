# coding: utf-8
# apigen/reg/generate.py  (reconstruido desde foto)

import os
import sys
import argparse
import jinja2 as jinja
import importlib
import re

GEN_PATH = os.path.dirname(__file__)
PROJECT_ROOT = os.path.abspath(os.path.join(GEN_PATH, '..'))
CWD = os.getcwd()

TEMPLATES = {
    'block': [
        ['h_api_gen.jinja',   '{}_{}_api.h'],
        ['c_api_gen.jinja',   '{}_{}_api.c'],
        ['c_gen.jinja',      '{}_{}.c'],
        ['py_api_gen.jinja',  '{}_{}_api.py'],
    ],
    'group': [
        ['h_api_modules_gen.jinja', '{}_modules.h'],
        ['c_api_modules_gen.jinja', '{}_modules.c'],
    ],
    'global': [
        ['py_api_dbg_hdr_gen.jinja', 'api_dbg_hdr.py'],
        ['py_api_init_gen.jinja',    '__init__.py'],
    ],
}

# Resolver paths de las plantillas a disco
for t in TEMPLATES.values():
    for i, (path, _) in enumerate(t):
        t[i][0] = os.path.join(PROJECT_ROOT, 'tt', path)

def parse_function_params(func_data):
    p_list, p_names, py_in, py_out = [], [], [], []
    param_count = 0
    arg_type = 0

    for i, (param, p_data) in enumerate(func_data['params'].items()):
        p_list.append('void' if param == '_void' else '{} {}'.format(p_data['type'], param))
        if param != '_void':
            param_count += 1
            p_names.append(param)
            arg_type |= ((0 if p_data['type'] == 'uint32_t' else 1) << i)
            p_data['arg_number'] = i
            try:
                p = p_data['len']
                if 'in' in p_data['dir']:
                    func_data['params'][p]['src'] = param
            except KeyError:
                pass

            m = re.search(r'(\w+)\[?(\d+)?]?', p_data['type'])
            p_data['c_type'] = re.sub(r'\[\d+\]', '', p_data['type'].replace('__', ''))
            p_data['base_type'] = m.group(1)
            if m.group(2):
                p_data['size'] = int(m.group(2))
            else:
                p_data['size'] = 1 if 'out' in p_data['dir'] else 0
        else:
            p_data['base_type'] = ''
            p_data['size'] = 0
    
    for param, p_data in func_data['params'].items():
        if param != '_void':
            if 'in' in p_data['dir'] and not 'src' in p_data:
                py_in.append(param)
            p_data['in'] = 'in' in p_data['dir']
            if 'out' in p_data['dir']:
                p_data['out'] = True
                py_out.append(param)
            else:
                p_data['out'] = False

    func_data['param_count']  = param_count        # cantidad
    func_data['param_list']   = ', '.join(p_list)  # prototipo C
    func_data['param_names'] = p_names           # lista nombres
    func_data['py_in_params'] = py_in              # entrada PY
    func_data['py_out_params'] = py_out            # salida PY
    func_data['arg_type']     = arg_type           # bitfield TLV

def parse_struct(struct_data):
    s_list, s_names = [], []
    for i, (s_param, s_data) in enumerate(struct_data['fields'].items()):
        s_list.append('{} {}'.format(s_data['type'], s_param))
        s_names.append(s_param)
        try:
            p = s_data['len']
            if 'in' in s_data['dir']:
                struct_data['params'][p]['src'] = s_param
        except KeyError:
            pass

        m = re.search(r'(\w+)\[?(\d+)?]?', s_data['type'])
        s_data['c_type'] = re.sub(r'\[\d+\]', '', s_data['type'].replace('__', ''))
        s_data['base_type'] = m.group(1)
        if m.group(2):
            s_data['size'] = int(m.group(2))
        else:
            s_data['size'] = 1

def parse_block(block, main_prefix, block_name, base, api):
    """
    Construye un dict cómodo para Jinja con prefijos y nombres base.
    """
    data = {
        'package_name': api.package_name,
        'main_prefix' : main_prefix,
        'block_name'  : block_name,
        'base'        : base,
        'elements'    : block,
    }
    fn_count = 0
    enum_count = 0
    struct_count = 0
    define_count = 0
    for name, elem in block.items():
        if elem['type'] == 'function':
            elem['base'] = fn_count 
            parse_function_params(elem)
            fn_count += 1
        elif elem['type'] == 'enum':
            enum_count += 1
        elif elem['type'] == 'struct':
            struct_count += 1
            parse_struct(elem)
        elif elem['type'] == 'define':
            define_count += 1
    data['fn_count'] = fn_count
    data['enum_count'] = enum_count
    data['struct_count'] = struct_count
    data['define_count'] = define_count
    api.api[main_prefix][block_name] = data
    return data

def parse_group(main_prefix, blocks, api):
    section = {
        'main_prefix': main_prefix,
        'block_count': len(blocks),
        'blocks': blocks.keys(),
    }
    api[main_prefix] = { 'block_count': len(blocks), 'blocks': blocks }
    return section

def parse_input(filename, out_dir):
    import_name = os.path.splitext(filename)[0]
    api_map = importlib.import_module(import_name)
    if not hasattr(api_map, 'templates'):
        api_map.templates = TEMPLATES
    if out_dir:
        api_map.output_dirs={ os.path.basename(f).replace('.jinja', ''): '{}'.format(out_dir) for v in TEMPLATES.values() for f, _ in v }
    if not hasattr(api_map, 'output_dirs'):
        raise AttributeError('El módulo de entrada debe definir "output_dirs" o pasarse con -o/--out-dir')
    block_start = getattr(api_map, 'group_start', 0)
    for main_prefix, blocks in api_map.api.items():
        for i, (block_name, block) in enumerate(blocks.items()):
            data = parse_block(block, main_prefix, block_name, block_start + i, api_map)
            for template in api_map.templates['block']:
                block_outdir = api_map.output_dirs[os.path.basename(template[0]).replace('.jinja', '')]
                outfile = os.path.join(block_outdir, (template[1].format(main_prefix, block_name)).lower())
                render_output(data, outfile, template[0])
        section = parse_group(main_prefix, blocks, api_map.api)
        for template in api_map.templates.get('group', []):
            section_outdir = api_map.output_dirs[os.path.basename(template[0]).replace('.jinja', '')]
            section_outfile = os.path.join(section_outdir, (template[1].format(main_prefix)).lower())
            render_output(section, section_outfile, template[0])
        block_start += api_map.group_size
    for template in api_map.templates['global']:
        g_outdir = api_map.output_dirs[os.path.basename(template[0]).replace('.jinja', '')]
        outfile = os.path.join(g_outdir, template[1])
        render_output(api_map.api, outfile, template[0])

def render_output(data, outfile, template):
    template_dir = os.path.dirname(template)
    template_file = os.path.basename(template)
    jinja_env = jinja.Environment(loader=jinja.FileSystemLoader(template_dir))
    jinja_env.trim_blocks = True
    jinja_env.lstrip_blocks = True
    t = jinja_env.get_template(template_file)
    s = t.render(data=data)
    os.makedirs(os.path.dirname(outfile), exist_ok=True)
    print('Generando:', outfile)
    with open(outfile, 'wb') as f:
        f.write(s.encode('utf-8'))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in-file', action='store', dest='in_file', 
                        required=True, help='Archivo de entrada .py con la definición del API')
    parser.add_argument('-o', '--out-dir', action='store', dest='out_dir', default=None,
                        help='Directorio de salida para todos los archivos generados (opcional)')
    args = parser.parse_args()
    sys.path.append(CWD)

    parse_input(args.in_file, args.out_dir)