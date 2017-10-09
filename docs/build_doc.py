import argparse, subprocess, platform, os, shutil, webbrowser

def clean():
    files = []
    folders = ['_build', 'doxygen/html', 'doxygen/latex', 'doxygen/xml']
    for f in files:
        if os.path.isfile(f):
            os.remove(f)

    for f in folders:
        if os.path.isdir(f):
            shutil.rmtree(f)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build_doxygen', action='store_true',
        default=False)
    parser.add_argument('--rtd', action='store_true',
        default=False, help='Simulate running on RTD')
    parser.add_argument('--clean', action='store_true',
        default=False, help='Remove build artifacts')

    args = parser.parse_args()
    is_build_doxygen = args.build_doxygen
    is_on_rtd = args.rtd
    is_clean = args.clean

    if is_clean:
        clean()
        return

    if is_build_doxygen:
        try:
            subprocess.call(['doxygen', 'Doxyfile'])
        except Exception as e:
            print('Failed to run doxygen')
            print(e)

    env = dict(os.environ)

    if is_on_rtd:
        env['READTHEDOCS'] = 'True'

    cmd = [
        'sphinx-build', '-n',
        '-b', 'html', # build format
        '.', '_build/html', # input, output folder
        '-a',
    ]
    print(cmd)
    subprocess.call(cmd, env = env)
    # subprocess.call(cmd, env = os.environ)

    index_file = os.path.join('_build', 'html', 'index.html')
    webbrowser.open_new(index_file)

if __name__ == '__main__':
    main()
