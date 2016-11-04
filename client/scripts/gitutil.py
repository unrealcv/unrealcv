import os

def is_git_dirty(wd):
    # print 'Check the git status of "%s"' % wd
    status = os.popen('git -C %s status -s' % wd).read().strip()
    if status != '':
        print 'Folder %s has uncommited changes' % wd
        print status
        return True
    return False

def get_git_tag(wd):
    # Get git tag of the working directory
    print 'Get git tag of "%s"' % wd
    if is_git_dirty(wd):
        return ""
    else:
        tag = os.popen('git -C %s describe --tags --exact-match' % wd).read().strip()
        return tag

def get_remote_info(wd):
    print 'Get remote info of "%s"' % wd
    # The the repo information of the working directory
    remote = os.popen('git config --get remote.origin.url').read().strip()
    return remote

def get_sha_version(wd):
    print 'Get sha version of "%s"' % wd
    if is_git_dirty(wd):
        return ""

    short_version = os.popen('git -C %s rev-parse --short HEAD' % wd).read().strip()

    return short_version
