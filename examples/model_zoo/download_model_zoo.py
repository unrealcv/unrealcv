import subprocess, os
from urllib.request import urlretrieve
platform = 'Windows'
version = 'master'
ids = ['RealisticRendering', 'ArchinteriorsVol2Scene1', 'ArchinteriorsVol2Scene2', 'ArchinteriorsVol2Scene3', 'UrbanCity']

if __name__ == '__main__':
    for binary_id in ids:
        filename = '{id}-{platform}-{version}.zip'.format(id = binary_id, platform = platform, version = version)
        url = 'http://www.cs.jhu.edu/~qiuwch/release/unrealcv/{filename}'.format(filename = filename)
        if not os.path.isfile(filename):
            print('Download from %s to %s' % (url, filename))
            # subprocess.call(['wget', '-c', url])
            try:
                urlretrieve(url, filename)
            except:
                print('Fail to download %s' % url)
        else:
            print('Ignore downloaded file %s' % filename)
