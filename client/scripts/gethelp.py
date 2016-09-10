import sys, time
from unrealcv import client

def main():
    client.connect()
    res = client.request('vget /unrealcv/help')

    # format response
    lines = res.strip().split('\n')
    uris = lines[0::2]
    shorthelp = lines[1::2]

    content = format_table(uris, shorthelp)
    help_file = 'help.md'
    with open(help_file, 'w') as f:
        f.write(content)
        print 'Help for UnrealCV commands is saved to %s' % help_file

def format_table(uris, shorthelp):
    row_template = '| %-60s| %-80s|'
    table = ''
    head = row_template % ('URI', 'Help')
    split = '|:' + 60 * '-' + '|:' + 80 * '-' + '|'
    rows = []
    for rowid in range(len(uris)):
        rows.append(row_template % (uris[rowid], shorthelp[rowid]))
    content = '\n'.join([head, split] + rows)
    return content


if __name__ == '__main__':
    main()
