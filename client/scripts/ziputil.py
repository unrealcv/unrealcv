import os

# Even empty folder needs to be preserved
def get_all_files(files):
    all_files = []
    for path in files:
        # if os.path.isfile(path):
        all_files.append(path)

        if os.path.isdir(path):
            for dirname, subdirs, files in os.walk(path):
                for filename in files:
                    all_files.append(os.path.join(dirname, filename))

                for subdir in subdirs:
                    all_files.append(os.path.join(dirname, subdir))

    return [os.path.abspath(v) for v in all_files]

def zipfiles(srcfiles, srcroot, dst):
    print 'zip file'
    zf = zipfile.ZipFile("%s" % (dst), "w", zipfile.ZIP_DEFLATED)

    for srcfile in srcfiles:
        arcname = srcfile[len(srcroot) + 1:]
        print 'zipping %s as %s' % (srcfile,
                                    arcname)
        zf.write(srcfile, arcname)

    zf.close()
