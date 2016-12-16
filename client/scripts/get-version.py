from gitutil import GitUtil
from osutil import OSUtil
import sys
# print 'unknown'

def get_git_version():
    git = GitUtil('.')
    if (git.is_dirty()):
        return 'dirty'

    version = git.get_tag()
    if not version:
        version = git.get_sha_version()

    return version

def get_os():
    os = OSUtil()
    return os.platform()

if __name__ == '__main__':
    # Redirect all print to stderr
    stdout = sys.stdout
    sys.stdout = sys.stderr

    platform = get_os()
    git_version = get_git_version()
    version ='%s-%s' % (git_version, platform)
    stdout.write(version)


