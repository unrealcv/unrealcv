#!/bin/bash
project_file=$1
plugin_version=$(git -C . rev-parse --short HEAD)
python build-plugin.py
python install-plugin.py ${plugin_version} ${project_file}
python build-project.py ${project_file}
