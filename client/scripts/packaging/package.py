if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    # parser.add_argument('project_file', required=True)
    parser.add_argument('project_file')
    args = parser.parse_args()

    project_file = os.path.abspath(args.project_file).replace('/drives/d/', 'D:/').replace('/home/mobaxterm/d/', 'D:/')
    package(conf, project_file)
