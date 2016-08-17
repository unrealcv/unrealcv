import os

def is_dirty(git_repo):
    git_repo = os.path.abspath(git_repo)
    status = os.popen('git -C %s status -s' % git_repo).read().strip()
    if status != '':
        print 'Folder %s has uncommited changes' % git_repo
        print status
        return True
    return False

def get_short_version(project_folder):
    short_version = os.popen('git -C %s rev-parse --short HEAD' % project_folder).read().strip()
    return short_version
