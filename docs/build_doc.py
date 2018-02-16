import argparse, subprocess, platform, os, shutil, webbrowser
script_folder = os.path.dirname(os.path.realpath(__file__))

def clean():
    files = []
    folders = ['_build', 'doxygen/html', 'doxygen/latex', 'doxygen/xml']
    for f in files:
        if os.path.isfile(f):
            os.remove(f)

    for f in folders:
        if os.path.isdir(f):
            shutil.rmtree(f)

class DoxygenBuilder:
    def __init__(self):
        pass
    
    def run(self):
        p = subprocess.Popen(['doxygen', 'Doxyfile'], cwd = script_folder)
        p.wait()

class WebpageViewer:
    def open_file(self, filename):
        webbrowser.open_new('file://' + os.path.realpath(filename))

    def open(self, url):
        webbrowser.open_new(url)

class SphinxBuilder:
    def __init__(self):
        self.env = dict(os.environ)
        self.on_rtd = False
        self.doc_folder = script_folder 
        self.output_folder = os.path.join(self.doc_folder, '_build/html')
        self.cmd = [
            'sphinx-build', '-n',
            '-b', 'html', # build format
            self.doc_folder, self.output_folder, # input, output folder
            '-j', '16', # build in parallel
        ]
        self.full_build = False
        pass

    def build_doxygen_meta(self):
        try:
            subprocess.call(['doxygen', 'Doxyfile'])
        except Exception as e:
            print('Failed to run doxygen')
            print(e)

    def lfs_checkout(self):
        import git_lfs
        doc_dir = os.path.dirname(os.path.abspath(__file__))
        project_dir = os.path.dirname(doc_dir)
        git_lfs.fetch(project_dir)

    def run(self):
        cmd = self.cmd
        if self.full_build:
            cmd += ['-a']

        if self.on_rtd:
            self.lfs_checkout()
            self.env['READTHEDOCS'] = 'True' # A string
        p = subprocess.Popen(cmd, env = self.env, cwd = script_folder)
        p.wait()

def argparser():
    parser = argparse.ArgumentParser()
    
    parser.add_argument('task', help='Which build task to run')

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
    return parser

def main():
    parser = argparser()

    args = parser.parse_args()
    tasks = [args.task]

    viewer = WebpageViewer()

    if 'doxygen' in tasks:
        doxygen_builder = DoxygenBuilder()
        doxygen_builder.run()
        viewer.open_file(os.path.join(script_folder, 'doxygen/html/index.html'))

    if 'sphinx' in tasks:
        sphinx_builder = SphinxBuilder()
        sphinx_builder.full_build = True
        sphinx_builder.run()
        viewer.open_file(os.path.join(script_folder, '_build/html/index.html'))

if __name__ == '__main__':
    main()
