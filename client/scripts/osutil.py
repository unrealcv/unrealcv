import sys

class OSUtil():
    def __init__(self):
        pass

    def platform(self):
        platform = sys.platform # Map from python platform name to ue4 platform name
        names = {
            'cygwin': 'Win', # could be win32 also
            'win32': 'Win',
            'win64': 'Win',
            'linux2': 'Linux',
            'darwin': 'Mac',
            }
        return names[platform]
