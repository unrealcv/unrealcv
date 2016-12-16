import os

class GitUtil:
    def __init__(self, wd):
        self.wd = wd

    def is_dirty(self):
        print 'Check the git status of "%s"' % self.wd
        status = os.popen('git -C %s status -s' % self.wd).read().strip()
        if status != '':
            print 'Folder %s has uncommited changes' % self.wd
            print status
            return True
        return False

    def get_tag(self):
        # Get git tag of the working directory
        print 'Get git tag of "%s"' % self.wd
        if self.is_dirty():
            return ""
        else:
            tag = os.popen('git -C %s describe --tags --exact-match' % self.wd).read().strip()
            return tag

    def get_remote_info(self):
        print 'Get remote info of "%s"' % self.wd
        # The the repo information of the working directory
        remote = os.popen('git config --get remote.origin.url').read().strip()
        return remote

    def get_sha_version(self):
        print 'Get sha version of "%s"' % self.wd
        if self.is_dirty():
            return ""

        short_version = os.popen('git -C %s rev-parse --short HEAD' % self.wd).read().strip()

        return short_version
