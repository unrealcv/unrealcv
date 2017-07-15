import argparse, subprocess, platform, os

def open_url(url):
    ''' Utility function '''
    system_name = platform.system()
    if system_name == 'Darwin':
        browser = 'open'
    elif system_name == 'Linux':
        browser = 'xdg-open'
    elif system_name == 'Windows':
        browser = 'explorer.exe'
    else:
        return
    subprocess.call([browser, url])


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--build_tutorial', action='store_true',
        default=False, help='Whether we should run the ipython notebook')
    parser.add_argument('--build_doxygen', action='store_true',
        default=False)
    parser.add_argument('--rtd', action='store_true',
        default=False, help='Simulate running on RTD')

    args = parser.parse_args()
    is_build_tutorial = args.build_tutorial
    is_build_doxygen = args.build_doxygen
    is_on_rtd = args.rtd

    if is_build_doxygen:
        try:
            subprocess.call(['doxygen', 'Doxyfile'])
        except Exception as e:
            print('Failed to run doxygen')
            print(e)

    env = dict(os.environ)
    # print(env)
    if is_build_tutorial:
        env['UNREALCV_BUILD_TUTORIAL'] = 'True'

    if is_on_rtd:
        env['READTHEDOCS'] = 'True'

    cmd = [
        'sphinx-build', '-n',
        '-b', 'html', # build format
        '.', '_build/html', # input, output folder
    ]
    print(cmd)
    subprocess.call(cmd, env = env)
    # subprocess.call(cmd, env = os.environ)

    index_file = os.path.join('_build', 'html', 'index.html')
    open_url(index_file)
