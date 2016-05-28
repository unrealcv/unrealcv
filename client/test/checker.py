import time, os
has_plt = False
try:
    import matplotlib.pyplot as plt
    has_plt = True
except:
    pass

def validate_format(response):
    valid = True
    valid &= (response == response.strip())
    return valid

# TODO: Check the async operation, make sure the task is already completed.
def justcreated(filename, timelimit=1):
    flag = True

    flag &= validate_format(filename)

    # Make sure the file is just created
    now = time.time()
    t = os.path.getctime(filename)
    delta = now - t
    flag &= (delta > 0) # file is created before this function
    flag &= (delta < timelimit) # file is just created
    return flag

def ispng(response):
    flag = True
    flag &= validate_format(response)
    flag &= response.endswith('.png')
    # Make sure file exsit
    flag &= os.path.isfile(response)

    # Make sure file readable
    if has_plt:
        im = plt.imread(response)

    # Make sure file is just created
    flag &= justcreated(response)
    return flag

def skip(response):
    flag = True
    flag &= validate_format(response)
    return flag
