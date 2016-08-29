#!/bin/bash
project_file=$1
plugin_version=$(git -C . rev-parse --short HEAD)
echo "Build plugin"
python build-plugin.py
echo "Install plugin to project"
python install-plugin.py ${plugin_version} ${project_file}
echo "Build project"
python build-project.py ${project_file}
