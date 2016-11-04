# This script is used to automatically release unrealcv plugin to github releases
# This filename contains '-', meaning it should not be imported into any other script
from build_plugin import build_plugin
from zip_folder import zip_folder
from release_to_github import release_to_github
import os, argparse
import gitutil, ue4util

build_folder = 'tmp/built_plugin'
parser = argparse.ArgumentParser()
parser.add_argument('--plugin', required=True, help='The plugin file to release')
parser.add_argument('--engine', required=True, help='UE4 install path')
args = parser.parse_args()

github_username = os.environ['github_user']
github_password = os.environ['github_password']

plugin_folder = os.path.dirname(os.path.abspath(args.plugin))
tag_version = gitutil.get_git_tag(plugin_folder)
platform = ue4util.get_platform_name()

folder = os.path.join(build_folder, '%s' % tag_version)
zip_file = os.path.join('unrealcv-%s-%s.zip' % (tag_version, platform))

if not os.path.isfile(zip_file):
    build_plugin(plugin=args.plugin, engine=args.engine, output=folder, release=True)
    zip_folder(folder, zip_file)

release_to_github(user=github_username, password=github_password, repo='qiuwch/unrealcv', file=zip_file, tag=tag_version, ignore_check=True)
# We already did check in previous step
