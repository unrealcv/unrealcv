def iserror(res):
    return (res is None) or res.startswith('error')

def isok(res):
    return res == 'ok'
