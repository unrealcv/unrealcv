from unrealcv.automation import UE4Automation, get_platform_name, get_plugin_version
import os, json, zipfile

def main():
    platform_name = get_platform_name()

    ue4_win = r"C:\Program Files\Epic Games\UE_4.16"
    ue4_linux = "/home/qiuwch/workspace/UE416"
    ue4_mac = '/Users/Shared/Epic Games/UE_4.16'

    ue4 = dict(Win64 = ue4_win, Linux = ue4_linux, Mac = ue4_mac)
    ue4_path = ue4.get(platform_name)

    abs_descriptor_file = os.path.abspath('./UnrealCV.uplugin')
    if not os.path.isfile(abs_descriptor_file):
        print('Can not find the plugin descriptor')
        return

    plugin_version = get_plugin_version(abs_descriptor_file)
    zip_filename = 'unrealcv_%s_%s.zip' % (platform_name, plugin_version)

    output_folder = 'Plugins/UnrealCV'
    abs_output_folder = os.path.abspath(output_folder)

    ue4 = UE4Automation(ue4_path)
    ue4.build_plugin(abs_descriptor_file, abs_output_folder, overwrite = True)
    zip_dir(output_folder, zip_filename)


def zip_dir(dirpath, zippath):
    # fzip = zipfile.ZipFile(zippath, 'w', zipfile.ZIP_DEFLATED, allowZip64 = True)
    fzip = zipfile.ZipFile(zippath, 'w', zipfile.ZIP_STORED, allowZip64 = True)
    basedir = os.path.dirname(dirpath) + '/'
    for root, dirs, files in os.walk(dirpath):
        if os.path.basename(root)[0] == '.':
            continue #skip hidden directories
        dirname = root.replace(basedir, '')
        for f in files:
            # if f[-1] == '~' or (f[0] == '.' and f != '.htaccess'):
            #     #skip backup files and all hidden files except .htaccess
            #     continue
            fzip.write(root + '/' + f, dirname + '/' + f)
    fzip.close()

if __name__ == '__main__':
    main()
