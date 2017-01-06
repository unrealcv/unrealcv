def convert_abspath(f):
    ''' This is a utility function to convert windows path to cygwin path '''
    f = f.replace('\\', '/')
    f = f.replace('C:/', '/mnt/c/')
    f = f.replace('D:/', '/mnt/d/')
    return f
