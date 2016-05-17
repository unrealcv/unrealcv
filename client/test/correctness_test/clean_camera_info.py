import os, shutil

if __name__ == '__main__':
    filename = './camera_info.txt'
    out_filename = './camera_info_basename.txt'
    with open(filename, 'r') as f:
        lines = f.readlines()

    with open(out_filename, 'w') as f:
        for line_id in range(len(lines)):
            line = lines[line_id].strip() # Remove \n at the end
            if line_id % 3 == 0: # filename
                # Copy file to this folder and convert absolute path to rel path
                fullfilename = line
                basename = os.path.basename(fullfilename)
                shutil.copy(fullfilename, basename)
                line = basename
            f.write(line + '\n')
