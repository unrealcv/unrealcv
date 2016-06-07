from ue4config import conf
import argparse, os

bin = os.path.join(conf['EnginePath'], 'Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh')
bin_dir = os.path.dirname(bin)
print bin_dir

def generate_makefile(conf, projectfile):
    cmd = '{bin} -project={projectfile} -game'.format(
        bin = bin,
        projectfile = projectfile
    )
    os.chdir(bin_dir)
    os.system(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')

    args = parser.parse_args()

    project_file = args.project_file
    generate_makefile(conf, project_file)
