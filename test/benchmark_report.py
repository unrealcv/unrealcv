'''
Make a report showing the speed of some critical commands
Check whether it is fast enough for your application
TODO: Add machine configuration, os, UnrealCV version, etc.
'''
from __future__ import print_function
from unrealcv import client
import time, json, argparse

def sys_info():
    def _unrealcv_info():
        plugin_descriptor = '../UnrealCV.uplugin'
        with open(plugin_descriptor) as f:
            description = json.load(f)
        plugin_version = description['VersionName']
        return plugin_version

    def _platform_info():
        import platform
        return platform.system()

    return 'Platform: {platform}     UnreaCV: {unrealcv}'.format(
        platform = _platform_info(),
        unrealcv = _unrealcv_info()
    )

def run_count(cmd, count):
    tic = time.time()
    for i in range(count):
        res = client.request(cmd)
        if i == 0:
            sample_output = res

    toc = time.time()
    elapse = toc - tic
    msg = 'Run %s for %d times, time = %.2f, %d FPS' \
        % (cmd, num, elapse, float(num) / elapse)
    return msg, sample_output

def run_duration(cmd, duration):
    tic = time.time()
    for i in range(100000):
        elapse = time.time() - tic # A little overhead here
        if elapse > duration:
            count = i
            break

        res = client.request(cmd)
        if i == 0:
            sample_output = res

    msg = 'Run %s for %0.2f seconds, count = %d, %d FPS' \
        % (cmd, duration, count, float(count) / duration)
    return msg, sample_output

def main():
    def log(msg):
        # no need to use logging module for such a simple use case
        print(msg)
        report_file.write(msg + '\n')

    parser = argparse.ArgumentParser()
    parser.add_argument('--task', default='benchmark_task.json', help = 'A json file to define the task')
    parser.add_argument('--report', default='benchmark_report.txt')

    args = parser.parse_args()
    json_task = args.task
    report_filename = args.report

    try:
        data = json.load(open(json_task))
    except Exception as e:
        print('Failed to parse task file %s' % json_task)
        print(e)
        return
    tasks = data.get('Tasks')

    report_file = open(report_filename, 'w')
    log(sys_info())

    client.connect()
    for task in tasks:
        task_text = str(task)
        if task.get('Cmd') == None or task.get('Skip') == True:
            continue

        if task.get('Duration'):
            duration = task.get('Duration')
            task_result, sample_output = run_duration(task['Cmd'], duration)

        if task.get('Count'):
            count = task.get('Count')
            task_result, sample_output = run_count(task['Cmd'], count)

        sample_output = sample_output.split('\n')[0][:256]
        log(task_text)
        log(task_result)
        log(sample_output)
        log('-' * 80)

    report_file.close()

if __name__ == '__main__':
    main()
