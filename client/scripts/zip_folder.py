import argparse, os
import ziputil
# This script is trying to mimic the function of zip [zipfile] -r [folder]
# The implementation of this script is mainly for supporting the automation in windows

def zip_folder(folder, zip_file, debug=False):
    folder = os.path.abspath(folder)
    files = ziputil.get_all_files([folder])
    ziputil.zipfiles(files, folder, zip_file, verbose=debug)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--folder', required=True, help='The folder to compress')
    parser.add_argument('--zip_file', required=True, help='The zip file to produce')
    parser.add_argument('--debug', action='store_true')

    args = parser.parse_args()
    zip_folder(args.folder, args.zip_file, args.debug)

if __name__ == '__main__':
    main()
