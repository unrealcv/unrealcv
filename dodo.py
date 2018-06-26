def task_clean():
    cmd = 'rm -rf Binaries/ Intermediate/'
    return {
        'actions': [cmd],
        'verbosity': 2,
    }
