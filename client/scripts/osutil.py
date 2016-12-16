import sys, platform

class OSUtil():
    def __init__(self):
        pass

    def platform(self):
        win = 'Win'
        mac = 'Mac'
        linux = 'Linux'
        if platform.release().endswith('Microsoft'):
            # This is a hacky way to check whether I am running Ubuntu on Windows
            return win

        # Map from python platform name to ue4 platform name
        names = {
            'cygwin': win, # could be win32 also
            'win32': win,
            'win64': win,
            'linux2': linux,
            'darwin': mac,
            }
        return names[sys.platform]
