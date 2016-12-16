# This is a script to push build artifact to github release
# This script depends on requests, uritemplate.py, python-magic
# This script can be either called from command line or import by another python script, both provide the same api
import argparse, json, os
import gitutil
import requests
from pprint import pprint

class GithubUtil:
    def __init__(self, repo, auth):
        self.base_url = 'https://api.github.com/repos/'
        self.repo_url = repo
        self.auth = auth
        self.git = gitutil.GitUtil(os.path.curdir)

    def get(self, url, **kwargs):
        print 'GET API: %s' % url
        kwargs['auth'] = self.auth
        return requests.get(url, **kwargs)

    def post(self, url, **kwargs):
        print 'POST API: %s' % url
        kwargs['auth'] = self.auth
        return requests.post(url, **kwargs)

    @classmethod
    def get_content_type(cls, filename):
        try:
            import magic
            mime = magic.Magic(mime=True)
            mime_type = mime.from_file(filename)
        except:
            print 'import magic failed, use hand-made solution'
            print 'Filename %s' % filename
            splits = filename.split('.'); ext = splits[-1]
            # The list is from here.
            mime_mapping = {
                'zip': 'application/zip'
            }
            return mime_mapping[ext]

        return mime_type

    def query_release_by_tag(self, tag_name):
        print 'Get release with tag %s from repo %s' % (tag_name, self.repo_url)
        url = self.base_url + self.repo_url + '/releases/tags/' + tag_name
        r = self.get(url)
        if r.status_code in [200]: # Successful
            data = json.loads(r.text)
            return data
        else:
            print r.text
            return None

    def list_all_release(self):
        url = self.base_url + self.repo_url + '/releases'
        r = self.get(url)
        release_data = json.loads(r.text)
        pprint(release_data)
        return r

    def upload_asset(self, release_id, filename):
        basefilename = os.path.basename(filename)
        url = 'https://uploads.github.com/repos/'+ self.repo_url + '/releases/%s/assets?name=%s' % (release_id, basefilename)

        content_type = self.get_content_type(filename)
        args = {
            'Content-Type': content_type, # What if I don't set this manually?
        }
        f = open(filename, 'rb')
        r = self.post(url, data=f, headers=args)
        print 'Upload asset %s to release %s of %s' % (filename, release_id, self.repo_url)
        print r.text

        return r

    def release(self, filename):
        '''
        Release a file based on the git version
        '''
        tag_version = self.git.get_tag()
        if not tag_version:
            print 'This version is not correctly tagged, nothing to release'
            return

        release = self.query_release_by_tag(tag_version)
        if not release:
            print 'There is not a release for this tag version, please create it first'
            return

        self.upload_asset(release['id'], filename)

    def upload(self, filename, tag_name, ignore_check=False):
        # Check remote info is what we want
        remote = self.git.get_remote_info()

        if not ignore_check:
            is_https_suffix = remote.endswith('/%s.git' % self.repo_url)
            is_git_suffix = remote.endswith(':%s.git' % self.repo_url)
            if not is_https_suffix and not is_git_suffix:
                print 'Working directory is not a part of %s' % repo_url
                return False

            # Check current commit is what we want
            tag = git.get_tag()
            if tag != tag_name:
                print 'Working directory is tagged with "%s", not "%s"' % (tag, tag_name)
                return False

        release = self.query_release_by_tag(tag_name)
        if not release:
            print 'The release with tag %s can not be found' % tag_name
            return False

        self.upload_asset(release['id'], filename)

def main():
    parser = argparse.ArgumentParser(description='Upload artifact to github release')
    parser.add_argument('--username', required=True, help='Username of github')
    parser.add_argument('--password', required=True, help='Password of github')
    parser.add_argument('--repo', required=True, help='Repo to push to, format (username/repo)')
    parser.add_argument('--filename', required=True, help='File to push')
    parser.add_argument('--tag', required=False, help='The tag to push to')
    parser.add_argument('--ignore_check', action='store_true', help='Do not check whether we are in the correct repo and tag')

    args = parser.parse_args()

    github_util = GithubUtil(args.repo, (args.username, args.password))
    if args.tag:
        github_util.upload(args.filename, args.tag)
    else:
        github_util.release(args.filename)

if __name__ == '__main__':
    main()
