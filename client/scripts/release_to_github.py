# This is a script to push build artifact to github release
# This script depends on requests, uritemplate.py, python-magic
# This script can be either called from command line or import by another python script, both provide the same api
import argparse, json, os
import gitutil
import requests
from pprint import pprint

base_url = 'https://api.github.com/repos/'
auth = ('username', 'password')

def get(url, **kwargs):
    print 'GET API: %s' % url
    kwargs['auth'] = auth
    return requests.get(url, **kwargs)

def post(url, **kwargs):
    print 'POST API: %s' % url
    kwargs['auth'] = auth
    return requests.post(url, **kwargs)

def main():
    parser = argparse.ArgumentParser(description='Upload artifact to github release')
    parser.add_argument('--user', required=True, help='Username of github')
    parser.add_argument('--password', required=True, help='Password of github')
    parser.add_argument('--repo', required=True, help='Repo to push to, format (username/repo)')
    parser.add_argument('--file', required=True, help='File to push')
    parser.add_argument('--tag', required=True, help='The tag to push to')
    parser.add_argument('--ignore_check', action='store_true', help='Do not check whether we are in the correct repo and tag')

    args = parser.parse_args()
    release_to_github(args.user, args.password, args.repo, args.file, args.tag, args.ignore_check)

def release_to_github(user, password, repo, file, tag, ignore_check=False):
    global auth
    auth = (user, password)

    # list_all_release(repo_url = args.repo)
    push(
        repo_url = repo,
        tag_name = tag,
        filename = file,
        ignore_check = ignore_check,
    )

def get_content_type(filename):
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

def query_release_by_tag(repo_url, tag_name):
    print 'Get release with tag %s from repo %s' % (tag_name, repo_url)
    url = base_url + repo_url + '/releases/tags/' + tag_name
    r = get(url)
    if r.status_code in [200]: # Successful
        data = json.loads(r.text)
        return data
    else:
        print r.text
        return None

def list_all_release(repo_url):
    url = base_url + repo_url + '/releases'
    r = get(url)
    release_data = json.loads(r.text)
    pprint(release_data)
    return r

def upload_asset(repo_url, release_id, filename):
    basefilename = os.path.basename(filename)
    url = 'https://uploads.github.com/repos/'+ repo_url + '/releases/%s/assets?name=%s' % (release_id, basefilename)

    content_type = get_content_type(filename)
    args = {
        'Content-Type': content_type, # What if I don't set this manually?
    }
    f = open(filename, 'rb')
    r = post(url, data=f, headers=args)
    print 'Upload asset %s to release %s of %s' % (filename, release_id, repo_url)
    print r.text

    return r


def push(repo_url, tag_name, filename, ignore_check):
    # Check remote info is what we want
    remote = gitutil.get_remote_info(os.path.curdir)

    if not ignore_check:
        is_https_suffix = remote.endswith('/%s.git' % repo_url)
        is_git_suffix = remote.endswith(':%s.git' % repo_url)
        if not is_https_suffix and not is_git_suffix:
            print 'Working directory is not a part of %s' % repo_url
            return False

        # Check current commit is what we want
        tag = gitutil.get_git_tag(os.path.curdir)
        if tag != tag_name:
            print 'Working directory is tagged with "%s", not "%s"' % (tag, tag_name)
            return False

    release = query_release_by_tag(repo_url, tag_name)
    if not release:
        print 'The release with tag %s can not be found' % tag_name
        return False

    upload_asset(repo_url, release['id'], filename)

if __name__ == '__main__':
    main()
