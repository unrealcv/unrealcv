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
    parser.add_argument('--build_doxygen',
        action='store_true', default=False
    )

    parser.add_argument('--rtd',
        action='store_true', default=False,
        help='Simulate running on RTD server'
    )

    parser.add_argument('--clean',
        action='store_true', default=False,
        help='Remove build artifacts'
    )

    parser.add_argument('--rebuild',
        action='store_true', default=False,
        help='Rebuild all the files to see all the warnings. By default only diffs are built to save time.'
    )
    # build diff takes about less than 1 second
    # a full rebuild takes about 5 minutes

    args = parser.parse_args()
    is_build_doxygen = args.build_doxygen
    is_on_rtd = args.rtd
    is_clean = args.clean
    rebuild = args.rebuild

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

    doc_folder = os.path.dirname(os.path.realpath(__file__))
    output_folder = os.path.join(doc_folder, '_build/html')
    cmd = [
        'sphinx-build', '-n',
        '-b', 'html', # build format
        doc_folder, output_folder, # input, output folder
        '-j', '16', # build in parallel
    ]

    if rebuild:
        clean()
        # cmd.append('-a')
    print(cmd)
    subprocess.call(cmd, env = env)
    # subprocess.call(cmd, env = os.environ)

    index_file = os.path.join(output_folder, 'index.html')
    print('Open compiled html in the browser')
    webbrowser.open_new('file://' + os.path.realpath(index_file))

if __name__ == '__main__':
    main()
