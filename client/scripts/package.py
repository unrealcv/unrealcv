# Automatically package game content
from package_config import *
import argparse

def package(project_file, output_directory):
    platform = 'Linux'  # Automatically detect platform
    cmd = '{RunUAT} BuildCookRun -project={project_file} -archivedirectory={output_directory} -noP4 -platform={platform} -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'
    cmd = cmd.format(RunUAT=UATScript, project_file=project_file, output_directory=output_directory, platform=platform)
    print(cmd)
    os.system(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--project_file', required=True)
    parser.add_argument('--output_directory', required=True)
    args = parser.parse_args()

    package(args.project_file, args.output_directory)
