#!/bin/bash
# reference: http://stackoverflow.com/questions/149057/how-to-remove-trailing-whitespace-of-all-files-recursively
git grep -I --name-only -z -e '' | xargs -0 sed -i 's/[ \t]\+\(\r\?\)$/\1/'

