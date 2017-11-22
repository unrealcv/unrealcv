# Build script of unrealcv, supports win, linux and mac.
# Weichao Qiu @ 2017
# Use python build.py to build the plugin
import argparse
from unrealcv.automation import UE4Automation
import os

def main():
    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'descriptor_file',
        default='UnrealCV.uplugin',
        nargs='?',
        help='The uplugin file to build'
    )
    parser.add_argument(
        '--install',
        action='store_true'
    )
    parser.add_argument(
        '--output',
        help='Output folder for this script'
    )
    parser.add_argument(
        '--UE4',
        help='Specify the engine path. If left empty, default installation locations will be used'
    )
    parser.add_argument(
        '--overwrite',
        type=bool,
        help='Whether to overwrite the compiled binary'
    )

    args = parser.parse_args()
    need_install = args.install

    ue4 = UE4Automation(args.UE4)

    if args.output:
        output_folder = args.output
    else:
        output_folder = None

    # Build the plugin
    descriptor_file = args.descriptor_file
    abs_descriptor_file = os.path.abspath(descriptor_file)
    if descriptor_file.endswith('.uplugin'):
        if not output_folder:
            output_folder = 'Plugins/UnrealCV'
        abs_output_folder = os.path.abspath(output_folder)

        ue4.build_plugin(abs_descriptor_file, abs_output_folder, args.overwrite)

        # Install the plugin if requested
        if need_install:
            ue4.install(plugin_folder = abs_output_folder, overwrite = True)

    elif descriptor_file.endswith('.uproject'):
        project_name = os.path.basename(descriptor_file).split('.')[0]
        if not output_folder:
            output_folder = 'UE4Binaries/%s' % project_name
        abs_output_folder = os.path.abspath(output_folder)

        ue4.package(abs_descriptor_file, abs_output_folder)

if __name__ == '__main__':
    main()
